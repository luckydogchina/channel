# channel

## 目的
模仿了golang中channel的机制，主要是用于多线程之间的数据传递，目前与golang channel有所差距：
+ 不支持select，众所周知golang的channel是支持select监听的
+ 不能跨进程传输，当然golang中channel也不支持
本项目的channel是比较适用于生产者-消费者模式。

## 使用说明
channel有多种配置：
1. 是否有容量限制，这是为了避免当消费者“死亡”或“过载”而导致任务拥积；
2. 是否有超时等待，如果不设置超时等待，消费者会一直阻塞，直到新任务到来。

