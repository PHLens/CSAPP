#include<stdio.h>
#include<unistd.h>
#include<getopt.h>

int main(int argc, char* argv[])
{
    char c;	
   /* for(int i=0; i<argc; i++){
    	printf("%s\n",argv[i]);
    }*/
    while((c=getopt(argc,argv,"a:b:cd"))!=-1){
    	printf("c=%c\t\t",c);
	printf("optarg = %s\t\t",optarg);
	printf("optind = %d\t\t",optind);
	printf("argv[optind] = %s\n",argv[optind]);
    }
    
    return 0;		
}
