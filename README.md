# myShell

代码已上传：https://github.com/MGABronya/myShell

编译示例

````shell
gcc myShell.c -o myShell -lreadline
````

已实现功能：

## 1.prompt

打印出提示用户输入的信息

主要使用的函数：

- gethostname， 用于取出主机名

- getpwuid取出用户相关信息

- getcwd获取当前目录

示例如下

![img](http://img.mgaronya.com/78c9a95b15a47cbe3543fd3b08524d1.png) 

## 2.help指令

并没有什么大用，当前只是用来打印MGAronya，嘿嘿，计划用来打印该文档的信息

 ![img](http://img.mgaronya.com/a3630e646273009f8529b089a7131c9.png) 

## 3.exit指令

用于退出程序

- exit，用于退出程序

 示例如下![img](http://img.mgaronya.com/6c72ffd6c75ba2542c5d9f136a116b9.png) 

## 4.cd指令

切换当前工作目录

主要使用的函数：

-  getpwuid，用于获取一个passwd结构体，从而获取用户名以及该用户的home目录
- chdir，用于切换工作目录

  示例图如下![img](http://img.mgaronya.com/33244724b7c7d52cc8f259244463785.png) 

## 5.管道指令

执行一个管道命令，如 `a指令 | b指令`，a指令的输出将作为b指令的输入

主要使用的函数：

- pipe，用于创建管道，供b指令传输信息给a指令
- fork，用于创建出子进程。首先会创建执行a指令的子进程，之后执行a指令的进程将会创建执行b指令的子进程。
- execvp，用于执行指令，即a指令和b指令
- waitpid，用于父进程等待子进程

   示例图如下![img](http://img.mgaronya.com/54c67062d8a40972159ec473dd84577.png) 

## 6.重定向指令

执行一个重定向指令，如`a指令 > b文件`，a指令的输出将输入到b文件内

主要使用的函数：

- fork，用于创建出子进程。创建执行a指令的子进程。
- execvp，用于执行指令，即a指令
- wait，用于等待进程
- fopen，打开指定文件
- freopen，将流与文件关联

​    示例图如下![](http://img.mgaronya.com/c56515b97cec00879bb7219d5649742.png) 

## 7.普通指令

对于普通指令，将用一个子进程完成

主要使用的函数：

- fork，用于创建出子进程。创建执行普通指令的子进程。
- execvp，用于执行指令，即普通指令