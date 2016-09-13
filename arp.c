/*
 * arp.c
 *
 * Kandice McLean
 * #300138073
 * 
 * Awesomely awesome arp implementation. A+ material.
 */

#include<stdlib.h>
#include <stdio.h>
#include <string.h>
#include "arp.h"
#include "ethernet.h"


struct map *addresses[MAX_ADDRESSES]; //Address Book
int num_maps = 0; //Current number of addresses in book

/*Create an arp request type and send off to ethernet layer*/
void send_arp_request(CnetAddr cnetAddress){
    
    struct arphdr* request = malloc(sizeof(struct arphdr));

    request->op_code = ARP_REQUEST;
    memcpy(request->source_mac, linkinfo[1].nicaddr , sizeof(CnetNICaddr));
    memcpy(&request->source_ip, &nodeinfo.address, sizeof(CnetAddr));
    memcpy(&request->dest_ip, &cnetAddress, sizeof(CnetAddr));
    CNET_parse_nicaddr(request->dest_mac, BROADCAST_MAC);
    request->HTYPE = ETHER;
    request->PTYPE = ARP;
    request->HLEN = ETHER_LEN;
    request->PLEN = ARP_LEN;
    
    ethernet_send(request->dest_mac, ETHERTYPE_ARP, (char*)request, sizeof(struct arphdr));
    free(request);
     
}

/*Create an arp reply type and send off to ethernet layer*/
void send_arp_reply(struct arphdr* arpptr){

    arpptr->op_code = ARP_REPLY;
    arpptr->HTYPE = ETHER;
    arpptr->PTYPE = ARP;
    arpptr->HLEN = ETHER_LEN;
    arpptr->PLEN = ARP_LEN;
    memcpy(&arpptr->dest_mac, &arpptr->source_mac, sizeof(CnetNICaddr));
    memcpy(&arpptr->dest_ip, &arpptr->source_ip, sizeof(CnetAddr));
    memcpy(arpptr->source_mac, linkinfo[1].nicaddr , sizeof(CnetNICaddr));
    memcpy(&arpptr->source_ip, &nodeinfo.address, sizeof(CnetAddr));

    ethernet_send(arpptr->dest_mac, ETHERTYPE_ARP, (char*)arpptr, sizeof(struct arphdr));
   
}

/*Accept a packet from ethernet.c, check if its for us, then
check if its reply or request and deal with data appropriatley*/
void arp_accept(char *packet, size_t length){
    
    struct arphdr *arpptr;
    arpptr = (struct arphdr*) packet;   
   
    if(arpptr->op_code==ARP_REQUEST){
    
        if(nodeinfo.address==arpptr->dest_ip){          
            send_arp_reply(arpptr);           
        }
        if(addresses_contains(arpptr->source_ip)==-1){
            insert_new_map(arpptr);         
        }     
  
    }    
    else if(arpptr->op_code==ARP_REPLY){        

        if(nodeinfo.address==arpptr->dest_ip){            
            insert_new_map(arpptr);            
        }

    }
    //dont care about it when it gets to here   
   
}

/*Insert a new map into the address book, or if already there, update ttl*/
void insert_new_map(struct arphdr *arpptr){

    if(arpptr->source_ip==nodeinfo.address){
        return;}

    int index = addresses_contains(arpptr->source_ip);


    if(index==-1){
        struct map *new_map = malloc(sizeof(struct map));
        memcpy(&new_map->ip_addr,&arpptr->source_ip, sizeof(CnetAddr));
        memcpy(&new_map->mac_addr,&arpptr->source_mac,sizeof(CnetNICaddr));
        new_map->ttl = nodeinfo.time_in_usec;

        if(num_maps==MAX_ADDRESSES){
            delete_used_maps(1);            
        }
        addresses[num_maps] = new_map;    
        num_maps++;    
    }
    else{
        addresses[index]->ttl = nodeinfo.time_in_usec;
    }

}

/*Called everytime a packet is received, clears out old addresses that haven't been used in EXPIRY time*/
void delete_used_maps(int removal_type){

    int index, i;
    if(removal_type==1){
        index = 0;
        for(i = 1; i < MAX_ADDRESSES; i++){
            if(addresses[index]->ttl>=addresses[i]->ttl){
                index = i;
            }
        }
        addresses[index] = NULL;
        if(index!=MAX_ADDRESSES-1){
            addresses[index] = addresses[MAX_ADDRESSES-1];
            addresses[MAX_ADDRESSES-1] = NULL;         
        }
        num_maps--;      
    }
    else{
        long long int time_now = nodeinfo.time_in_usec;
        for(i = 0; i < num_maps; i++){ 
            long long int expire = time_now-addresses[i]->ttl;   
            if(expire>=EXPIRATION){
                addresses[i] = NULL;
                addresses[i] = addresses[num_maps-1];
                num_maps--;               
            }
        }
    }

}

/*Check if an address is already in the address book*/
int addresses_contains(CnetAddr cnetAddress){    
    int i;    
    for(i = 0; i < num_maps; i++){        
        if(addresses[i]->ip_addr == cnetAddress){
            return i;
        }    
    }
    return -1;
}

/*assigns a macAddress and returns true if map is available, otherwise 
sends a request and returns false*/
bool arp_get_mac_address(CnetAddr cnetAddress, CnetNICaddr macAddress)
{
    print_maps();
    int index = addresses_contains(cnetAddress);
    if(index!=-1){       
        char *mac = malloc(sizeof(CnetNICaddr));
        CNET_format_nicaddr(mac, addresses[index]->mac_addr);
        CNET_parse_nicaddr(macAddress, mac);
        return true;
    }
    else{
        send_arp_request(cnetAddress);
        return false;
    } 

}

/*print functions for debugging purposes*/

void print_maps(){
    printf("\n****************************************\n");
    printf("\nPrinting %d Maps in Address Book of NODE: %d\n\n",num_maps,nodeinfo.address);
    int i = 0;    
    for(i = 0; i < num_maps; i++){
        print_one_map(addresses[i]);
    }
    printf("\n****************************************\n");
}

void print_one_map(struct map* m){
    char *mac = malloc(sizeof(CnetNICaddr));
    CNET_format_nicaddr(mac, m->mac_addr);
    printf("The MAC for Node %d is %s\n",m->ip_addr,mac);
    printf("Its time to live started at %lld seconds\n\n",m->ttl);
}

void print_arp(struct arphdr* arpptr){

    char *source_mac = malloc(sizeof(CnetNICaddr));
    char *dest_mac = malloc(sizeof(CnetNICaddr));
    CNET_format_nicaddr(source_mac, arpptr->source_mac);
    CNET_format_nicaddr(dest_mac, arpptr->dest_mac);

    fprintf(stderr,"HTYPE = %hu \n",arpptr->HTYPE);
    fprintf(stderr,"PTYPE = %hu \n",arpptr->PTYPE);
    fprintf(stderr,"HLEN = %d \n",arpptr->HLEN);
    fprintf(stderr,"PLEN = %d \n",arpptr->PLEN);
    fprintf(stderr,"opcode = %hu \n",arpptr->op_code);
    fprintf(stderr,"MAC source = %s \n",source_mac);
    fprintf(stderr,"MAC dest = %s \n",dest_mac);
    fprintf(stderr,"ip source = %d \n",arpptr->source_ip);
    fprintf(stderr,"ip dest = %d \n",arpptr->dest_ip);
  
}


























