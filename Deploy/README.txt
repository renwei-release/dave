这个文件夹，是DAVE系统里面所有服务，统一部署脚本文件所在的位置。

所有产品均以Docker方式部署。

DAVE部署分成两种不同的部署模式：开发人员开发时部署，运维人员安装时部署。

1，开发人员开发时部署

C/Go
如果需要编译的语言，都会有一个与编译有关的统一文件目录，他们是
C/build/linux/[Product name]或者Go/build/[Product name]，每个[Product name]的
子目录下都会有一个build脚本，运行build脚本就可以编译对应的产品，编译完成后，
直接在该目录下，运行
./deploy 脚本开始在开发环境本地部署产品。
Python
如果是解释性的语言，则没有build相关操作，代码书写完成后，直接cd到
Python/deploy/[Product name]部署目录下，运行
./deploy 脚本开始在开发环境本地部署产品。

2，运维人员安装时部署

运维只需要获取Deploy文件夹下的文件。
运维同事可以把Deploy文件拷贝到需要做部署的机器上。比如，想部署[Product name]服务，
可以cd到Deploy/deploy/[Product name]目录下，运行./deploy.sh
就可以部署[Product name]服务了。不同的服务，只是进入部署脚本的路径不一样，
部署命令都是同一个./deploy.sh。

3，前面2个步骤的总结

至此，整个开发到运维部署的过程就结束了。他们的简单流程为：

开发时部署: 
对C/Go 开发->build->deploy
对Python 开发->deploy

运维时部署：
cd Deploy/deploy/[Product name] && ./deploy.sh