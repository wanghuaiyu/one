RT-Thread��WIFI����

��ʹ��ǰ�����helper.bin��fw.bin�ļ��ŵ���������ĸ�Ŀ¼������ʹ��armcc��Keil MDK���룻

Ҫ����AP����Ҫ�ڹ���ǰ���һЩ��Ϣ��AP��Ҫ���óɷǼ���ģʽ����������
AP�������ŵ��ţ�channel����MAC��ַ����SSID

��wlan_main.c�ļ��У��޸�WlanDirectConnect�����ģ�
	int channel = 6;
	char *ssid = "pxa920";
	char mac[6] = {0x12, 0x34, 0x56, 0x2d, 0x08, 0xc9};

	wlan_set_ap_info(channel, mac);
	wlan_set_security(NoSecurity, ssid, RT_NULL);

channel��AP�������ŵ��ţ�
mac��AP��MAC��ַ��
ssid��AP�㲥��SSID��

���±��룬Ȼ�����ص�RealTouch�ϾͿ��Կ���WIFI����AP�ɹ������RealTouch�ĵ�ַ��д��ȷ���Ϳ��Դ�PC��pingͨRealTouch��