/*
  Este programa se conecta a un servidor y asume que el RTT es
  el tiempo de la conexion
*/

  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <netdb.h>
  #include <sys/time.h>
  #include <math.h>

// Funcion que calcula la diferencia entre 2 tiempos (segundos y microsegundos)
 int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}



int main(int argc, char * argv[])
{
  if(argc != 4) {
    printf("Debe dar el host y el puerto donde conectarse\n");
    printf("./RTT <host> <port> <packets>\n");
    exit(-1);
  }

struct timeval startTime, endTime, diffTime;


  char *theHost = argv[1];
  int thePort = atoi(argv[2]);
  int numPackets = atoi(argv[3]);
//  printf("host: %s\n", theHost);
//  printf("port:  %d\n", thePort);
//  printf("packets: %d\n", numPackets);

  // Pedir al DNS que resuelva el IP del servidor
  struct hostent *he;

  if((he = gethostbyname(theHost)) == NULL) {
    printf("error in gethostbyname()\n");
    exit(-1);
  }
 // printf("Nombre del host: %s\n", he->h_name);
  char *theIP = inet_ntoa(*((struct in_addr *)he->h_addr));
 // printf("Direccion IP: %s\n", theIP);

  struct sockaddr_in stSockAddr;
  int Res;
  int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(atoi(argv[2]));
  Res = inet_pton(AF_INET, inet_ntoa(*((struct in_addr *)he->h_addr)) , &stSockAddr.sin_addr);

  if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
  else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }


  // Valores para calcular el RTT
  long EstimatedRTT = 0;
  long SampleRTT = 0;
  long SimpleAverage = 0;
  long SimpleAverageAux = 0;
  long DevRTT = 0;
  long TotalSum = 0;
  long TimeoutInterval = 0;

 /************ CALCULO DE RTT & TIMEOUT *********************/

  // printf("SampleRTT\tEstimatedRTT\tSimpleAverage\tTimeoutInterval\n");
  int i;


  for(i = 1; i < numPackets; i++) {

     // Start Timer
    gettimeofday(&startTime, NULL);
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
     // End timer
    gettimeofday(&endTime, NULL);
    // Calcula la diferencia (endTime - startTime)
    timeval_subtract(&diffTime, &endTime, &startTime);
    // Asume que es menor a un segundo...
    // (diffTime.tv_usec) = microsegundos!!

    // CALCULAR SampleRTT
    SampleRTT = diffTime.tv_usec / 1000;
    // CALCULAR EstimatedRTT
    EstimatedRTT = (1 - 0.125) * EstimatedRTT + 0.125 * SampleRTT;
    // CALCULAR SimpleAverage usando TotalSum
    SimpleAverageAux += SampleRTT;
    SimpleAverage = SimpleAverageAux / i;
    // CALCULAR Timeout usando DevRTT
    DevRTT = (1 - 0.25) * DevRTT + 0.25 * abs (SampleRTT - EstimatedRTT);
    TimeoutInterval = EstimatedRTT + 4 * DevRTT;

    printf("%ld\t%ld\t%ld\t%ld\n", SampleRTT, EstimatedRTT, SimpleAverage, TimeoutInterval);

    close(SocketFD);

   }
  return 0;
}
