void udp_read(void){
  Data_length=Udp.parsePacket();  //获取接收的数据的长度
  if(Data_length)  //如果有数据那么Data_length不为0，无数据Data_length为0
  {
    len = Udp.read(incomingPacket, 1023);  //读取数据，将数据保存在数组incomingPacket中
    if (len > 0)  //为了避免获取的数据后面乱码做的判断
    {
      incomingPacket[len] = 0;
    }
    message = incomingPacket;
    do
    {
      commaPosition = message.indexOf(',');//检测字符串中的逗号
      if(commaPosition != -1)//如果有逗号存在就向下执行
      {
         inString = message.substring(0,commaPosition);//打印出第一个逗号位置的字符串
         message = message.substring(commaPosition+1, message.length());//打印字符串，从当前位置+1开始
      }
      else
      { //找到最后一个逗号，如果后面还有文字，就打印出来
         inString = message;  
      }
      switch(channel_int){
        case 1:channel_1 = inString.toInt();channel_int++;break;
        case 2:channel_2 = inString.toInt();channel_int++;break;
        case 3:channel_3 = inString.toInt();channel_int++;break;
        case 4:channel_4 = inString.toInt();channel_int=1;break;
      }
    }while(commaPosition >=0);
  }
}
