#include <stdio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

void generateKeys(){
    EVP_PKEY*pkey=EVP_RSA_gen(1024);
    if(pkey==NULL){
        fprintf(stderr,"error: rsa gen\n");
        ERR_print_errors_fp(stderr);
        return;
    }
    FILE*fp=fopen("public.txt","wt");
    if(fp!=NULL){
        PEM_write_PUBKEY(fp,pkey);
        fclose(fp);
    }else{
        perror("file error");
    }
    fp=fopen("private.txt","wt");
    if(fp!=NULL){
        PEM_write_PrivateKey(fp,pkey,NULL,NULL,0,NULL,NULL);
        fclose(fp);
    }else{
        perror("file error");
    }
    EVP_PKEY_free(pkey);
}

int main(int argc, char* argv[]) 
{
    generateKeys();
    return 0;
}