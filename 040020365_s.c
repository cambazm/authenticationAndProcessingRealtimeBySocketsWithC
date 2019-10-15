/*************************************
 * Bilgisayar Haberlesmesi Odev-3    *
 *				     *
 * Ogrenci : MEHMET CAMBAZ           *
 * No      : 040020365               *
 * E-mail  : mehmet_cambaz@yahoo.com *
 *************************************/

/***sunucu***/

/*program yazilirken temel olarak derste gosterilen ornek kodlar kullanilmistir*/

#define closesocket close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdio.h>
#include <string.h>

#define BUFFSIZE	255
#define ISIMBOYU	15
#define TELBOYU		7
#define PROTOPORT       5193            /* varsayilan protokol port numarasi */
#define QLEN            0               
/* istek kuyruk uzunlugu = 0
   yani hic bir proses beklemeye alinamaz
   istemci istekleri icin fork ile yeni bir proses olusturulur,
   eski proses yeni istekleri bekler, her yeni istek icin yeni 
   bir proses olusturulur */



void basarisiz_recv();
void dosyada_sorun();

/****************************************************************************************
* Fonksiyon adi: main 						 			*
* Amac: istemci isteklerini karsilamak							*
* Giris parametreleri: kullanilacak protokol port no	 				*
* Cikis parametreleri: 0 (basarili sonlanma) veya 1 (basarisiz sonlanma)		*
* Algoritma: iteratif algoritma					 			*
****************************************************************************************/
int main(int argc, char *argv[])
{
        struct  hostent  *ptrh;  /* host tablosu */
        struct  protoent *ptrp;  /* protokol tablosu  */
        struct  sockaddr_in sad; /* IP adresi tutmak icin struct */
        struct  sockaddr_in cad; /* istemci adresi tutmak icin struct  */
        int     sd, sd2;         /* soket tanimlayicisi */ 
        int     port;            /* protokol port numarasi */
        int     alen;            /* adres uzunlugu */
        char    buf[BUFFSIZE]; 
	char	isim[ISIMBOYU], sifre[ISIMBOYU], tel[TELBOYU],
		dosyadanIsim[ISIMBOYU], dosyadanSifre[ISIMBOYU], dosyadanTel[TELBOYU];
	char    localhost[] =   "localhost";    /* varsayilan host adi */
	FILE	*fpKontrol, *fpKayit;
	char	kontrol[] = "kontrol.txt", kayit[] = "kayit.txt";
	char	tmp[50];
	int	temp;
	int	veriUzunlugu=0, i, n, tmm, f;

        memset((char *)&sad,0,sizeof(sad));  /* sockaddr struct ini temizle */
        sad.sin_family = AF_INET;            /* ailesini internet ata */
        sad.sin_addr.s_addr = INADDR_ANY; 

	/* arguman olarak port verilmezse varsayilan deger atanir */
        if (argc > 1) 
            port = atoi(argv[1]);   
	else 
	{
            port = PROTOPORT;
	    printf("\nvarsayilan port atandi: 5193\n");     
        }

        if (port > 0)                  
            sad.sin_port = htons((u_short)port);
        else 
	{                         
            fprintf(stderr,"yanlis port numarasi %s\n",argv[1]);
            exit(1);
        }
	
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

        if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) 
	{
            fprintf(stderr,"baglama islemi basarisiz\n");
            exit(1);
        }


	/* 
	   PAKET YAPISI:
	   veri tipi bosluk veri uzunlugu bosluk veri
	   ornegin 4. tipten 6 uzunlukta "mehmet" verisi
	   "4 6 mehmet" olarak gonderilir
	*/
	
	/* "Veri uzunlugu uyumsuz" mesaji pakette belirtilen veri uzunlugu 
	   ile gelen verinin uzunlugu tutmuyor anlamina gelir */
	
        while (1) 
	{
		//anne proses devamli yeni istekleri dinleyecek
	        if (listen(sd, QLEN) < 0) 
		{
            		fprintf(stderr,"dinleme islemi basarisiz\n");
            		exit(1);
        	}
        	alen = sizeof(cad);
		
                if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) 
		{
                    fprintf(stderr, "kabul islemi basarisiz\n");
                    exit(1);
                }
		
		f = fork();
		//fork yapilamazsa
		if(f==-1)
		{
			printf("Fork yapilamadi...\n");

			buf[0] = '5';
			buf[1] = '\0';
			veriUzunlugu = strlen("fork_islemi_basarisiz");
			sprintf (tmp, "%d", veriUzunlugu);
			strcat(buf, " ");
			strcat(buf, tmp);
			strcat(buf, " ");
			strcat(buf, "fork_islemi_basarisiz");

			send(sd2,buf,strlen(buf),0);	//hata mesaji yolla
			
			closesocket(sd2);
			closesocket(sd);
			return 1;
		}
		//olusturulan proses (cocuk) istemcinin islerini gorecek ve sonlanacak
		else if(f==0)
		{
			//istemciden gelecek mesaji al
			n = recv(sd2, buf, sizeof(buf), 0);
			buf[n] = '\0';
			if(n > 0) 
			{
				//onay istegi mesaji gelmisse
				if(buf[0]=='0')	
				{
					sscanf(buf,"%d %d %s %s",&temp,&veriUzunlugu,&isim,&sifre);
					if( veriUzunlugu!=(strlen(isim)+strlen(sifre)+1) )
						printf("Veri uzunlugu uyumsuz");
					else
					{
						//kontrol.txt acilamazsa
						if( (fpKontrol = fopen(kontrol, "r")) == NULL )
						{
							dosyada_sorun();
			
							buf[0] = '5';
							buf[1] = '\0';
							veriUzunlugu = strlen("kontrol_dosyasi_yok");
							sprintf (tmp, "%d", veriUzunlugu);
							strcat(buf, " ");
							strcat(buf, tmp);
							strcat(buf, " ");
							strcat(buf, "kontrol_dosyasi_yok");
			
							send(sd2,buf,strlen(buf),0);	//hata mesaji yolla
							
							closesocket(sd2);	//soketi kapat
							closesocket(sd);	//soketi kapat
							return 1;
						}
						
						tmm=0;
						//kontrol.txt dosyasinda verilen isim ve sifreyi ara, eslestir
						while( fscanf(fpKontrol, "%s", dosyadanIsim) > 0 )
						{
							fscanf(fpKontrol, "%s", dosyadanSifre);
							//verilen kullanici ismi ve sifre dogruysa onay ver
							if( strcmp(dosyadanIsim,isim)==0 && strcmp(dosyadanSifre,sifre)==0 )
							{
								tmm=1;
								break;
							}
						}
						//onay alindiysa istemciye telefonu aranacak ismi sor 
						if(tmm==1)
						{
							buf[0] = '1';
							buf[1] = '\0';
							
							send(sd2,buf,strlen(buf),0);	//onay mesaji yolla
						}
						//onay alinamazsa hata mesaji yolla
						else 
						{
							buf[0] = '5';
							buf[1] = '\0';
							veriUzunlugu = strlen("kullanici_ve_sifre_dogru_degil");
							sprintf(tmp, "%d", veriUzunlugu);
							strcat(buf, " ");
							strcat(buf, tmp);
							strcat(buf, " ");
							strcat(buf, "kullanici_ve_sifre_dogru_degil");
		
							send(sd2,buf,strlen(buf),0);	//hata mesaji yolla
						}
		
						fclose(fpKontrol);
						
						//istemciden gelecek mesaji al
						n = recv(sd2, buf, sizeof(buf), 0);
						buf[n] = '\0';	
						if(n > 0)
						{
							//telefon sorgulama mesaji geldiyse
							if(buf[0]=='3')
							{
								sscanf(buf,"%d %d %s",&temp,&veriUzunlugu,&isim);
								if(veriUzunlugu!=strlen(isim))
									printf("Veri uzunlugu uyumsuz");
								else
								{
									//kayit.txt acilamazsa
									if( (fpKayit = fopen(kayit, "r")) == NULL )
									{
										dosyada_sorun();
			
										buf[0] = '5';
										buf[1] = '\0';
										veriUzunlugu = strlen("kayit_dosyasi_yok");
										sprintf(tmp, "%d", veriUzunlugu);
										strcat(buf, " ");
										strcat(buf, tmp);
										strcat(buf, " ");
										strcat(buf, "kayit_dosyasi_yok");
			
										send(sd2,buf,strlen(buf),0);	//hata mesaji yolla
								
										closesocket(sd2);	//soketi kapat
										closesocket(sd);	//soketi kapat
										return 1;
									}
									tmm=0;
									
									//kayit.txt de gonderilen isme ait telefon numarasini ara
									while( fscanf(fpKayit, "%s", dosyadanIsim) > 0 )
									{
										fscanf(fpKayit, "%s", dosyadanTel);
										//verilen isme ait bir telefon kayitliysa
										if( strcmp(dosyadanIsim,isim)==0 )
										{
											tmm=1;
											break;
										}
									}
									//ilgili isim bulunduysa bulunan telefon numarasini yolla 
									if(tmm==1)
									{
										buf[0] = '4';
										buf[1] = '\0';
										veriUzunlugu = 7;
										sprintf(tmp, "%d", veriUzunlugu);
										strcat(buf, " ");
										strcat(buf, tmp);
										strcat(buf, " ");
										strcat(buf, dosyadanTel);
			
										send(sd2,buf,strlen(buf),0);	//cevap mesaji yolla
									}
									//eger kayit bulunamazsa hata mesaji yolla
									else 
									{
										buf[0] = '5';
										buf[1] = '\0';
										veriUzunlugu = strlen("telefon_kaydi_bulunamadi");
										sprintf(tmp, "%d", veriUzunlugu);
										strcat(buf, " ");
										strcat(buf, tmp);
										strcat(buf, " ");
										strcat(buf, "telefon_kaydi_bulunamadi");
			
										send(sd2,buf,strlen(buf),0);	//hata mesaji yolla
									}
			
									fclose(fpKayit);
								}
							}
						}
					}
	
				}
			}
			else
				basarisiz_recv();
			
			break;	//cocuk proses istemcinin isteklerini karsiladiktan sonra sonlanacak, bundan dolayi while dan cikar
		}		
                closesocket(sd2);
        }
		
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
/****************************************
* Fonksiyon adi: dosyada_sorun	 	*
* Amac: hata bilgisi ekrana cikartmak	*
* Giris parametreleri: yok	 	*
* Cikis parametreleri: yok		*
****************************************/
void dosyada_sorun()
{
	printf("Dosyaya ulasilamadi\n");
} 
