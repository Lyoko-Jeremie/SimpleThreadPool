# SimpleThreadPool
C++11 base std::async like simple thread pool implement.

基于C++11实现的仿std::async的简单线程池

编写的原因是某一天看到js有<code>async/await</code>语法且C#也有，
就想到C++有<code>std::async</code>/<code>std::futur.get/wait</code>，于是在使用中发现VS2015的async实现是单线程的，
不能满足随心所欲的发起异步任务加快处理速度的想法，再加上很久之前就想写一个通用线程池但因拖延症和
没想好怎么传任务和异常所以一直没有写，但当看到std::async实现能够将异常和结果传出来于是就起了兴趣来写这个东西。

简单地说因为种种原因的不爽和兴趣就自己实现了一个线程池式的std::async。 

Head-Only

可以直接代换std::async

完全由C++11特性实现，无外部依赖库

虽然只是在VS上编写的.但是我相信简单修改一些语法就能同样正常运行在g++上
