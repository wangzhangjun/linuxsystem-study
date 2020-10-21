#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>        /* See NOTES */
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#pragma pack(push, 1)
struct DNSHeader
{
    /* 1. 会话标识（2字节）*/
    unsigned short usTransID;        // Transaction ID

    /* 2. 标志（共2字节）*/
    unsigned char RD : 1;            // 表示期望递归，1bit
    unsigned char TC : 1;            // 表示可截断的，1bit
    unsigned char AA : 1;            // 表示授权回答，1bit
    unsigned char opcode : 4;        // 0表示标准查询，1表示反向查询，2表示服务器状态请求，4bit
    unsigned char QR : 1;            // 查询/响应标志位，0为查询，1为响应，1bit

    unsigned char rcode : 4;         // 表示返回码，4bit
    unsigned char zero : 3;          // 必须为0，3bit
    unsigned char RA : 1;            // 表示可用递归，1bit

    /* 3. 数量字段（共8字节） */
    unsigned short Questions;        // 问题数
    unsigned short AnswerRRs;        // 回答资源记录数
    unsigned short AuthorityRRs;     // 授权资源记录数
    unsigned short AdditionalRRs;    // 附加资源记录数
};
#pragma pack(pop)

typedef unsigned char BYTE;

bool SendDnsPack(unsigned short usID,
                  int *pSocket,
                  const char *szDnsServer,
                  const char *szDomainName)
{
    bool bRet = false;
    if (*pSocket == -1
        || szDomainName == NULL
        || szDnsServer == NULL
        || strlen(szDomainName) == 0
        || strlen(szDnsServer) == 0)
    {
        return bRet;
    }
    unsigned int uiDnLen = strlen(szDomainName);
    // 判断域名合法性，域名的首字母不能是点号，域名的
    // 最后不能有两个连续的点号
    if ('.' == szDomainName[0] || ( '.' == szDomainName[uiDnLen - 1]
          && '.' == szDomainName[uiDnLen - 2])
       )
    {
        return bRet;
    }
     /* 1. 将域名转换为符合查询报文的格式 */
    // 查询报文的格式是类似这样的：
    // 例如:www.google.com 转成：3www6google3com0
    //注意最后一定是0
    unsigned int uiQueryNameLen = 0;
    BYTE *pbQueryDomainName = (BYTE *)malloc(uiDnLen + 1 + 1);
    if (pbQueryDomainName == NULL)
    {
        return bRet;
    }
    // 转换后的查询字段长度为域名长度 +2
    memset(pbQueryDomainName, 0, uiDnLen + 1 + 1);
    // 下面的循环作用如下：
    // 如果域名为  jocent.me ，则转换成了 6 j o c e n t  ，还有一部分没有复制
    // 如果域名为  jocent.me.，则转换成了 6 j o c e n t 2 m e
    unsigned int uiPos    = 0;
    unsigned int i        = 0;
    for ( i = 0; i < uiDnLen; ++i)
    {
      if (szDomainName[i] == '.')
      {
          pbQueryDomainName[uiPos] = i - uiPos;
          if (pbQueryDomainName[uiPos] > 0)
          {
              memcpy(pbQueryDomainName + uiPos + 1, szDomainName + uiPos, i - uiPos);
          }
          uiPos = i + 1;
      }
    }

    // 如果域名的最后不是点号，那么上面的循环只转换了一部分
    // 下面的代码继续转换剩余的部分， 比如 2 m e
    if (szDomainName[i-1] != '.')
    {
      pbQueryDomainName[uiPos] = i - uiPos;
      memcpy(pbQueryDomainName + uiPos + 1, szDomainName + uiPos, i - uiPos);
      uiQueryNameLen = uiDnLen + 1 + 1;
    }
    else
    {
      uiQueryNameLen = uiDnLen + 1;
    }
    // 填充内容  头部 + name + type + class
    DNSHeader *PDNSPackage = (DNSHeader*)malloc(sizeof(DNSHeader) + uiQueryNameLen + 4);
    if (PDNSPackage == NULL)
    {
        //goto exit;
    }
    memset(PDNSPackage, 0, sizeof(DNSHeader) + uiQueryNameLen + 4);


    // 填充头部内容
    PDNSPackage->usTransID = htons(usID);  // ID
    PDNSPackage->RD = 0x1;   // 表示期望递归
    PDNSPackage->Questions = htons(0x1);  // 本文第一节所示，这里用htons做了转换

    // 填充正文内容  name + type + class
    BYTE* PText = (BYTE*)PDNSPackage + sizeof(DNSHeader);
    memcpy(PText, pbQueryDomainName, uiQueryNameLen);

    unsigned short *usQueryType = (unsigned short *)(PText + uiQueryNameLen);
    *usQueryType = htons(0x1);        // TYPE: A

    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN

    // 需要发送到的DNS服务器的地址
    sockaddr_in dnsServAddr = {};
    dnsServAddr.sin_family = AF_INET;
    dnsServAddr.sin_port = htons(53);  // DNS服务端的端口号为53
    dnsServAddr.sin_addr.s_addr = inet_addr(szDnsServer);

    // 将查询报文发送出去
    int nRet = sendto(*pSocket,
        (char*)PDNSPackage,
        sizeof(DNSHeader) + uiQueryNameLen + 4,
        0,
        (sockaddr*)&dnsServAddr,
        sizeof(dnsServAddr));
    if (-1 == nRet)
    {
        printf("DNSPackage Send Fail! \n");
        goto exit;
    }

    // printf("DNSPackage Send Success! \n");
    bRet = true;

// 统一的资源清理处
exit:
    if (PDNSPackage)
    {
        free(PDNSPackage);
        PDNSPackage = NULL;
    }

    if (pbQueryDomainName)
    {
        free(pbQueryDomainName);
        pbQueryDomainName = NULL;
    }

    return bRet;
}
void RecvDnsPack(unsigned short usId, int *pSocket )
{
    if (*pSocket == -1)
    {
        return;
    }
    char szBuffer[256] = {};        // 保存接收到的内容
    sockaddr_in servAddr = {};
    socklen_t iFromLen = sizeof(sockaddr_in);
    int iRet = recvfrom(*pSocket,
        szBuffer,
        256,
        0,
        (sockaddr*)&servAddr,
        &iFromLen);
    if (-1 == iRet || 0 == iRet)
    {
        printf("recv fail \n");
        return;
    }

    /* 解析收到的内容 */
    DNSHeader *PDNSPackageRecv = (DNSHeader *)szBuffer;
    unsigned int uiTotal       = iRet;        // 总字节数
    unsigned int uiSurplus     = iRet;  // 接受到的总的字节数
    // 确定收到的szBuffer的长度大于sizeof(DNSHeader)
    if (uiTotal <= sizeof(DNSHeader))
    {
        printf("接收到的内容长度不合法\n");
        return;
    }
    // 确认PDNSPackageRecv中的ID是否与发送报文中的是一致的
    if (htons(usId) != PDNSPackageRecv->usTransID)
    {
        printf("接收到的报文ID与查询报文不相符\n");
        return;
    }

    // 确认PDNSPackageRecv中的Flags确实为DNS的响应报文
    if ( 0x01 != PDNSPackageRecv->QR )
    {
        printf("接收到的报文不是响应报文\n");
        return;
    }
    // 获取Queries中的type和class字段
    unsigned char *pChQueries = (unsigned char *)PDNSPackageRecv + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for ( ; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus ) { ; } // 跳过Queries中的name字段
    ++pChQueries;
    --uiSurplus;
    if ( uiSurplus < 4 )
    {
        printf("接收到的内容长度不合法\n");
        return;
    }
    unsigned short usQueryType  = ntohs( *((unsigned short*)pChQueries) );
    pChQueries += 2;
    uiSurplus -= 2;

    unsigned short usQueryClass = ntohs( *((unsigned short*)pChQueries) );
    pChQueries += 2;
    uiSurplus -= 2;
    // 解析Answers字段
    unsigned char *pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal){
        // 跳过name字段（无用）
        if ( *pChAnswers == 0xC0 )  // 存放的是指针
        {
            if (uiSurplus < 2)
            {
                printf("接收到的内容长度不合法\n");
                return;
            }
            pChAnswers += 2;       // 跳过指针字段
            uiSurplus -= 2;
        }
        else        // 存放的是域名
        {
            // 跳过域名，因为已经校验了ID，域名就不用了
            for ( ; *pChAnswers && uiSurplus > 0; ++pChAnswers, --uiSurplus ) {;}
            pChAnswers++;
            uiSurplus--;
        }

        if (uiSurplus < 4)
        {
            printf("接收到的内容长度不合法\n");
            return;
        }

        unsigned short usAnswerType = ntohs( *((unsigned short*)pChAnswers) );
        pChAnswers += 2;
        uiSurplus -= 2;

        unsigned short usAnswerClass = ntohs( *( (unsigned short*)pChAnswers ) );
        pChAnswers += 2;
        uiSurplus -= 2;

        if ( usAnswerType != usQueryType || usAnswerClass != usQueryClass )
        {
            printf("接收到的内容Type和Class与发送报文不一致\n");
            return;
        }

        pChAnswers += 4;    // 跳过Time to live字段，对于DNS Client来说，这个字段无用
        uiSurplus -= 4;

        if ( htons(0x04) != *(unsigned short*)pChAnswers )
        {
            uiSurplus -= 2;     // 跳过data length字段
            uiSurplus -= ntohs( *(unsigned short*)pChAnswers ); // 跳过真正的length

            pChAnswers += 2;
            pChAnswers += ntohs( *(unsigned short*)pChAnswers );
        }
        else
        {
            if (uiSurplus < 6)
            {
                printf("接收到的内容长度不合法\n");
                return;
            }

            uiSurplus -= 6;
            // Type为A, Class为IN
            if ( usAnswerType == 1 && usAnswerClass == 1)
            {
                pChAnswers += 2;

                unsigned int uiIP = *(unsigned int*)pChAnswers;
                in_addr in = {};
                in.s_addr = uiIP;
                printf("IP: %s\n", inet_ntoa(in));

                pChAnswers += 4;
            }
            else
            {
                pChAnswers += 6;
            }
        }
    }
}

int main()
{
    int socketfd;
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd < 0)
    {
        printf("create socket fail!\n");
        return -1;
    }
    int nNetTimeout = 2000;
    // 设置发送时限
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
    // 设置接收时限
    setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout,sizeof(int));

    // 随机生成一个ID
    srand((unsigned int)time(NULL));
    unsigned short usId = (unsigned short)rand();

    //域名
    char szDomainName[256] = {};
    printf("输入要查询的域名：");
    scanf("%s", szDomainName);

    // 发送DNS报文，因为测试，这里就简单指定8.8.8.8作为查询服务器
    if (!SendDnsPack(usId, &socketfd, "8.8.8.8", szDomainName))
    {
        return -1;
    }
    // 接收响应报文，并显示获得的IP地址
    RecvDnsPack(usId, &socketfd);

    close(socketfd);

    return 0;
}
