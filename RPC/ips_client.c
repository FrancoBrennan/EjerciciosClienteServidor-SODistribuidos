#include "ips.h"

void ips_programa_1(char *host, char * dominio)
{
	CLIENT *clnt;
	struct retorno  *result_1;
	struct envia  ips_1_arg;

#ifndef	DEBUG
	clnt = clnt_create (host, IPS_PROGRAMA, VERSION_IPS_PROGRAMA, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

           strcpy( ips_1_arg.dominio , dominio ); //agregado 

	result_1 = ips_1(&ips_1_arg, clnt);
	if (result_1 == (struct retorno *) NULL) {
		clnt_perror (clnt, "call failed");
	}

        printf("recibido del servidor %s \n", result_1->retips); //agregago

#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}

int main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host dominio\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	ips_programa_1 (host,argv[2]);
            exit (0);
}

