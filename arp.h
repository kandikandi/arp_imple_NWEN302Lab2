/*
 * arp.h
 *
 * @Kandice MCLean 
 * #300138073
 *
 */

#ifndef _ARP_H_
#define	_ARP_H_

#include <cnet.h>

#define MAX_ADDRESSES 5 //max number of addresses in "book"

#define ARP_REQUEST 1
#define ARP_REPLY 2
#define BROADCAST_MAC "ff:ff:ff:ff:ff:ff" //flooding MAC address
#define ETHER 1
#define ETHER_LEN 6
#define ARP 0x806
#define ARP_LEN 4
#define EXPIRATION 10000000000000000 //number of seconds that an address can remain in the address book before being removed

//nodes in address book
struct map{
    CnetAddr ip_addr;
    CnetNICaddr mac_addr; 
    long long int ttl; //used to clear old nodes
};

//ARP header structure
struct arphdr{
    unsigned short HTYPE;
    unsigned short PTYPE;
    unsigned char HLEN;
    unsigned char PLEN;
    unsigned short op_code;
    CnetNICaddr source_mac;
    CnetAddr source_ip;
    CnetNICaddr dest_mac;
    CnetAddr dest_ip;
};


//function declarations
void send_arp_request(CnetAddr cnetAddress);
void send_arp_reply(struct arphdr* arpptr);
void arp_accept(char *packet, size_t length);
void insert_new_map(struct arphdr *arpptr);
void delete_used_maps(int removal_type);
int addresses_contains(CnetAddr cnetAddress);
bool arp_get_mac_address(CnetAddr cnetAddress, CnetNICaddr macAddress);
void print_maps();
void print_one_map(struct map* m);
void print_arp(struct arphdr* arpptr);

#endif	/* _ARP_H_ */

