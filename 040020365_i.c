/*************************************
 * Bilgisayar Haberlesmesi Odev-3    *
 *				     *
 * Ogrenci : MEHMET CAMBAZ           *
 * No      : 040020365               *
 * E-mail  : mehmet_cambaz@yahoo.com *
 *************************************/

/***istemci***/

/*program yazilirken temel olarak derste gosterilen ornek kodlar kullanilmistir*/

#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

#define BUFFSIZE	255
#define ISIMBOYU	15
#define TELBOYU		7
#define PROTOPORT       5193            /* varsayilan protokol numarasi */

void basarisiz_recv();

/************************************************************************
* Fonksiyon adi: main 						 	*
* Amac: kullanici adi ve sifre alarak sunucudan onay istemek,		*
* onaylanirsa telefon numarasi aranan ismi almak ve sonucu kullaniciya  *
* bildirmek								*
* Giris parametreleri: sunucu ipsi ve protokol port nosu	 	*
* Cikis parametreleri: 0 (basarili sonlanma) veya 1 (basarisiz sonlanma)*
* Algoritma: iteratif algoritma					 	*
*************************************************************************/
int main(int argc, char *argv[])
{
        struct  hostent  *ptrh;			/* host tablosu */
        struct  protoent *ptrp;			/* protokol tablosu  */
        struct  sockaddr_in sad; 		/* IP adresi tutmak icin struct */
        struct  sockaddr_in cad; 		/* istemci adresi tutmak icin struct  */
        int     sd;              		/* soket tanimlayicisi */
        int     port;            		/* protokol port numarasi */
        char    *host;           		/* host ismi */
        int     i, n, isimBoyu=0, sifreBoyu=0, veriUzunlugu=0, temp;        
	int     alen;            		/* adres uzunlugu */
	char	isim[ISIMBOYU], sifre[ISIMBOYU], tel[TELBOYU];
	char    localhost[] =   "localhost";    /* varsayilan host adi */
	char    buf[BUFFSIZE];
	char	tmp[50], hata[50];


        memset((char *)&sad,0,sizeof(sad)); /* sockaddr struct ini temizle */
        sad.sin_family = AF_INET;           /* ailesini internet ata */

	/* arguman olarak port verilmezse varsayilan deger atanir */
        if (argc > 2) 
            port = atoi(argv[2]);  
	else
	{
            port = PROTOPORT;       /* varsayilan port numarasini kullan */
	    printf("varsayilan port atandi: 5193\n");
        }

        if (port > 0)
            sad.sin_port = htons((u_short)port);
        else 
	{
            fprintf(stderr,"yanlis port numarasi %s\n",argv[2]);
            exit(1);
        }

        /* sunucu adi/ipsi arguman olarak verilmezse varsayilan atanir */
        if (argc > 1) 
            host = argv[1];
	else
	{
            host = localhost;
	    printf("varsayilan host atandi: localhost\n\n");
        }

        /* sokette ip kismini host un ip adresiyle doldur */
        ptrh = gethostbyname(host);
        if ( ((char *)ptrh) == NULL ) 
	{
            fprintf(stderr,"gecersiz host: %s\n", host);
            exit(1);
        }
        memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

        /* protokol numarasini ilistir */
        if ( ((int)(ptrp = getprotobyname("tcp"))) == 0) 
	{
            fprintf(stderr, " \"tcp\" protokole ilistirilemedi");
            exit(1);
        }

        /* soket olustur */
        sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
        if (sd < 0) 
	{
            fprintf(stderr, "soket olusturulmasi basarisiz\n");
            exit(1);
        }

        /* belirtilen sunucuya baglan */
        if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) 
	{
            fprintf(stderr,"baglanti kurulamadi\n");
            exit(1);
        }

	/* 
	   PAKET YAPISI:
	   veri tipi bosluk veri uzunlugu bosluk veri
	   ornegin 4. tipten 6 uzunlukta "mehmet" verisi
	   "4 6 mehmet" olarak gonderilir
	*/
	
	/*
	   HATA TIPLERI:
	   kontrol dosyasi yok
	   kayit dosyasi yok
	   fork islemi basarisiz
	   kullanici ve sifre dogru degil
	   telefon kaydi bulunamadi	   
	*/

	/* "Veri uzunlugu uyumsuz" mesaji pakette belirtilen veri uzunlugu 
	   ile gelen verinin uzunlugu tutmuyor anlamina gelir */
		
	/****KULLANICIDAN ISIM VE SIFRE AL****/
	printf("Kullanici ismi: ");
	scanf("%s", &isim);
		
	if( (isimBoyu = strlen(isim)) <= 0 )	//kullanicidan isim al
	{
		printf("\nGecersiz isim girildi\n");
		return 1;
	}
	printf("Sifre: ");
	scanf("%s", &sifre);
		
	if( (sifreBoyu = strlen(sifre)) <= 0 )	//kullanicidan sifreyi al
	{
		printf("\nGecersiz sifre girildi\n");
		return 1;
	}

	buf[0] = '0';	//onay istegi mesaji yollanacak
		
	sprintf(tmp, "%d", isimBoyu + sifreBoyu + 1);
	buf[1] = '\0';
	strcat(buf, " ");
	strcat(buf,tmp);
	strcat(buf, " ");
	strcat(buf,isim);
	strcat(buf," ");
	strcat(buf,sifre);

	send(sd,buf,strlen(buf),0);		//onay istegi mesaji yolla
	
	buf[0]='\0';	//buf i sifirla
        
	n = recv(sd, buf, sizeof(buf), 0);	//sunucundan gelecek mesaji bekle
	buf[n] = '\0';
	if(n > 0) 
	{
		/*eger hata paketi geldiyse hata tipini goster, duruma gore sonlan*/
		if(buf[0]=='5') 
		{
			sscanf(buf,"%d %d %s",&temp,&veriUzunlugu,&hata);
			printf("%s\n", hata);
			
			//gerekli dosyalar yoksa veya fork islemi basarisizsa islem yapilamaz, uygulamayi kapat
			if( strcmp(hata,"kayit_dosyasi_yok")==0 || strcmp(hata,"kontrol_dosyasi_yok")==0 || strcmp(hata,"fork_islemi_basarisiz")==0 )
			{
				closesocket(sd);	//soketi kapat
				return 1;
			}
		}
		else if(buf[0]=='1')	//onay mesaji geldiyse, telefonu aranacak ismi al
		{
			printf("Isim: ");
			scanf("%s", &isim);

			if( (isimBoyu = strlen(isim)) <= 0 )	//kullanicidan telefonu istenen ismi al
			{
				printf("\nGecersiz isim girildi\n");
				return 1;
			}
			else
			{
				buf[0] = '3';	//sorgu mesaji gonderilecek

				sprintf(tmp, "%d", isimBoyu);
				buf[1] = '\0';
				strcat(buf, " ");
				strcat(buf,tmp);
				strcat(buf, " ");
				strcat(buf,isim);
				
				send(sd,buf,strlen(buf),0);		//sorgu mesaji yolla

				n = recv(sd, buf, sizeof(buf), 0);	//sunucudan gelecek cevabi bekle
				buf[n] = '\0';
				
				if(n > 0) 
				{
					/*eger hata paketi geldiyse hata tipini goster, duruma gore sonlan*/
					if(buf[0]=='5') 
					{
						sscanf(buf,"%d %d %s",&temp,&veriUzunlugu,&hata);
						if(veriUzunlugu==strlen(hata))
							printf("%s\n", hata);
						else
							printf("Veri uzunlugu uyumsuz\n");
						
						//gerekli dosyalar yoksa veya fork islemi basarisizsa islem yapilamaz, uygulamayi kapat
						if( strcmp(hata,"kayit_dosyasi_yok")==0 || strcmp(hata,"kontrol_dosyasi_yok")==0 || strcmp(hata,"fork_islemi_basarisiz")==0 )
						{
							closesocket(sd);	//soketi kapat
							return 1;
						}							
					}
					else if(buf[0]=='4')		//cevap mesaji geldiyse
					{
						sscanf(buf,"%d %d %s",&temp,&veriUzunlugu,&tel);
						if(veriUzunlugu==strlen(tel))	
							printf("Telefon: %s\n", tel);		//yollanan telefonu kullaniciya goster
						else
							printf("Veri uzunlugu uyumsuz\n");
					}
				}
				else
					basarisiz_recv();
			}	
		}
	}
	else
		basarisiz_recv();


        closesocket(sd);	//soketi kapat

        return 0;
}


/****************************************
* Fonksiyon adi: basarisiz_recv 	*
* Amac: hata bilgisi ekrana cikartmak	*
* Giris parametreleri: yok	 	*
* Cikis parametreleri: yok		*
****************************************/
void basarisiz_recv()
{
	printf("Bilgi alma islemi basarisiz\n");
}

