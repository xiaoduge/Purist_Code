程序整合版本  2017/01/17建立

1、用户菜单增加开关是否显示进水电导率
2、循环时间最小值改为6min， TOC过程1次冲洗修改为160s，二次冲洗15s
3、TOC自动检测，大于50ms时显示TOC

4、20170516修改，定量取水长按键可连续增减取水量
   修改函数：void Disp_KeyHandler(int key,int state)；

5、TOC检测过程，上电后第一个循环7min，若无TOC电极第2个循环开始循环3min

2017-10-13:
6、开启TOC报警显示
B25 >300  :  "CHECK U PACK"
C > 2450mV:  "CHECK TOC SENSOR"
 P/B < 1.3:  "CHECK UV LAMP"

2017年12月11日
珍能修改后的主板，在连接串口设备调试时，最好将X15 RST和GND短接。调试完成后再去除短接。


