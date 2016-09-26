# oneproxy-monitor
在实现一个数据库的监控器，需要实现下面的接口，在本框架中提供了一个默认的直接转发的监控部分。<br/>
必须实现的接口：<br/>
1. is_currentDatabase: 这个类是用来选择数据库类的，当connection第一次接收到数据包时，通过此类来判断当前连接是否属于这个数据库的连接，如果是则返回true。<br/>
否则返回false。如果为true，则框架将从配置中读取数据库地址和端口，连接连接，同时转发数据到这个连接上面。<br/>
2. createInstance: 框架通过调用这个函数来创建对应的实例，建议不要使用单例模式来创建，因为在这个实例中有prepared的相关信息，不同的连接可能有相同的句柄。<br/>
3. destoryInstance: 当连接完成时，会调用此函数来释放相关的内存等。<br/>
4. get_packetType:根据包的数据返回包的类型，由于不同数据库包的格式不同，类型的位置和长度不同，故需要不同的数据库协议类自己实现此接口。<br/>
可选实现的接口：<br/>
1. protocol_front: 前端接收到的数据包，都会传递给这个函数，协议类可以通过重新实现这个函数来改变默认的处理方式。框架在调用这个函数后，就直接把数据包转发到服务端了。<br/>
2. protocol_backend: 后端接收到数据包，都会传递给这个函数，协议类可以通过重新实现这个函数改变默认的处理方式。框架在调用这个函数后，就直接把数据包转发到客户端了。<br/>
3. prehandle_frontPacket:默认实现是没有处理数据包的任何内容。如果需要针对接收到的前端数据包进行处理，则需要实现此函数。比如：当客户端的数据包特别大时，被客户 端分成了多个数据包发送到服务端，在进行统计前，需要针对这些数据包进行合并，那么就需要在此包中进行合并的逻辑。<br/>
4. prehandle_backendPacket:与prehandle_frontPacket相同。不同的点是，这个函数针对接收到的后端数据包<br/>
5. protocol_initFrontStatPacket: 默认实现只是把bufpointer的指针指向接收到的数据包。如果在进行执行处理函数前，需要针对数据进行修改，那么需要实现此函数。<br/>
6. protocol_initBackendStatPacket: 与protocol_initFrontStatPacket函数功能相同，不同点是，这个针对后端数据包。<br/>
7. protocol_clearFrontStatPacket:当执行完协议类注册的处理函数后，执行此函数。默认实现是空的。<br/>
8. protocol_clearBackendStatPacket:功能与protocol_clearFrontStatPacket相同，不同的是这个函数针对后端数据包的处理。<br/>

默认具有如下的功能：<br/>
1. 可以指定多个端口，不同的端口转发到不同的数据库。也就是通过一个监控器管理多个数据库<br/>
2. 可以通过配置控制前端连接数<br/>
3. 自动根据cpu数量设置线程个数<br/>
4. 支持windows和Linux。其中windows只支持select。Linux支持select和epoll，默认为epoll方式<br/>
5. FakeProtocol 是针对包直接进行转发。<br/>

目前平民软件基于此架构已经提供sql server， postgresql的监控功能。<br/>
监控效果可以看在线的mysql的监控效果：http://www.onexsoft.com:8080/<br/>
更多信息，请访问我们的官网：http://www.onexsoft.com 或者加入群：数据库监控 521095285。<br/>
