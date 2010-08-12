/* knowndomain.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * $Id$
 */

#include "../include/dnsmole.h"
    
#define FALSE 0

kdomain *add_domain(kdomain *new_domain,kdomain *search_domain,int level){

    kdomain *tdomain = search_domain;
    if(!strcmp(search_domain->name,"ROOT") && !search_domain->kd_child){
        search_domain->kd_child = new_domain;
        search_domain->next = NULL;
        search_domain->prev = NULL;
        search_domain->kd_child->prev = search_domain;
        tdomain = search_domain->kd_child;
    }
    else if(!strcmp(search_domain->name,"ROOT") && search_domain->kd_child){
        tdomain = search_domain->kd_child;
    }
    else if(!strcmp(search_domain->name,"TEMP")){
        new_domain->prev = search_domain->prev;
        search_domain->prev->kd_child = new_domain;
        tdomain = new_domain;
    }

    while(tdomain){
        if(!strcmp(tdomain->name,new_domain->name) && tdomain->kd_child){
            return tdomain->kd_child;
        }
        else if(!strcmp(tdomain->name,new_domain->name) && !tdomain->kd_child){
            if(level == 1){
                tdomain->kd_child = new_domain_structure("TEMP",-1);
                tdomain->kd_child->prev = tdomain;
                return tdomain->kd_child;
            }
            else 
                return tdomain;
            
        }
        else if(!tdomain->next){
            tdomain->next = new_domain;
            tdomain->next->prev = tdomain; 
            tdomain = tdomain->next;
        }
        else if(tdomain->next){
            tdomain = tdomain->next;
        }
    }
}

void delete_domain(kdomain *domain){
    domain->prev = domain->next;

    domain_child_free(domain->kd_child);
    if(domain->name) free(domain->name);
    if(domain->cname) free(domain->cname);
    free(domain);
}

void domain_child_free(kdomain *domain_free){
    
    if(domain_free){
        domain_child_free(domain_free->kd_child);
        if(domain_free->name) free(domain_free->name);
        if(domain_free->cname) free(domain_free->cname);
        domain_child_free(domain_free->next);
        free(domain_free);
    }
}

void domain_add_cname(char *domain_name,char *name,kdomain *root_domain){

     kdomain *temp_domain = search_domain(domain_name,root_domain,1);
    
    if((temp_domain->cname = malloc(strlen(name)*sizeof(char) + 1)) == NULL){
        fprintf(stderr,"[malloc] OOM\n"); exit(EXIT_FAILURE);
    }
    memcpy(temp_domain->cname,name,strlen(name)+1);
}

kdomain *search_domain(char *name,kdomain *root_domain,int search_type){

    pcre *re;
    char **split_structure = malloc(sizeof(char *) * 4);
    int count = 0, len_size = 0;
    unsigned int temp_hash = 0;
    kdomain *temp_domain = root_domain->kd_child;

    re = initialize_regex();
    split_domain(name,re,split_structure);
       
    if(!split_structure[0]){
        pcre_free(re); return (kdomain *) 0;  
    }
    
    while(temp_domain){
        temp_hash = 0;
        
        if((len_size = strlen(split_structure[count])) < 20)
            temp_hash = hash(split_structure[count],len_size);

        if(((temp_domain->domain_hash == temp_hash) || !temp_hash)){
            len_size = (temp_domain->name_length <= len_size ? temp_domain->name_length : len_size);
            if(!memcmp(temp_domain->name,split_structure[count],len_size)){
                if((temp_domain->suspicious == 0) && (search_type == 0)){
                    pcre_free(re); return temp_domain;
                }
                else if((count != 3) && (split_structure[count+1] != NULL)){
                    count++; 
                    temp_domain = temp_domain->kd_child;
                } 
                else if(count != 3 && split_structure[count+1] == NULL){
                    return temp_domain;
                }
                else if(count == 3){
                    return temp_domain;
                }
            }
            else
                temp_domain = temp_domain->next;
        }
        else 
            temp_domain = temp_domain->next;
        
    }
    free(split_structure);
    pcre_free(re);
    return (kdomain *)0;
}

kdomain *new_domain_structure(char *name,int suspicious){
    
    kdomain *tmp_domain;

    if((tmp_domain = (kdomain *) malloc(sizeof(kdomain))) != NULL){
        if((tmp_domain->name = malloc(strlen(name) * sizeof(char) +1)) == NULL){
           fprintf(stderr,"[malloc] OOM\n"); exit(EXIT_FAILURE); 
        }
        memcpy(tmp_domain->name,name,strlen(name)+1);
        tmp_domain->kd_child = tmp_domain->next = tmp_domain->prev = NULL;
        tmp_domain->cname = NULL;
        tmp_domain->suspicious = suspicious;
        tmp_domain->name_length = strlen(name);
        tmp_domain->domain_hash = hash(name,tmp_domain->name_length);
    }

    else{ 
        fprintf(stderr,"[malloc] OOM\n"); exit(EXIT_FAILURE);
    }

    return tmp_domain;
}
    
void load_domain(char *line,pcre *re,kdomain *domain,int type){

    int vector[15]; 
    kdomain *temp_domain = domain, *new_domain;
    int i = 1;
    int splitcount;
    char **split_structure = malloc(sizeof(char *) * 4);;
    split_domain(line,re,split_structure);
    for(splitcount = 0; splitcount < 4;splitcount++){
        if(split_structure[splitcount] != NULL){
            if(split_structure[splitcount+1] == NULL){
                new_domain = new_domain_structure(split_structure[splitcount],type);
            }
            else
                new_domain = new_domain_structure(split_structure[splitcount],-1);

            if(split_structure[splitcount+1] == NULL)
                i = 0;
            temp_domain = add_domain(new_domain,temp_domain,i);
            free(split_structure[splitcount]);
        }
    }
    
    free(split_structure);
}

void split_domain(char *line,pcre *re,char **split_structure){
    
    int vector[15];
    char *substring, *nice_substring;
    int rc,i,substring_length;

    rc = pcre_exec(re,NULL,line,strlen(line),0,0,vector,15);

    for(i = rc-1; i >= 1; i--){
        substring = line + vector[2*i];
        substring_length = vector[2*i+1] - vector[2*i];
        nice_substring = strdup(substring); 
        *(nice_substring + substring_length) = '\0';
        if(substring_length){
            if(i == 1){ 
                nice_substring[strlen(nice_substring)-1] = '\0';
            }
            if((split_structure[rc-1 - i] = malloc(strlen(nice_substring)+1)) == NULL){
                fprintf(stderr,"[malloc] OOM\n"); exit(EXIT_FAILURE);
            }
            memcpy(split_structure[rc-1 - i],nice_substring,strlen(nice_substring)+1);
        }
        else {
            split_structure[rc-1 - i] = NULL;
        }
    }
}


        
pcre *initialize_regex(){
    pcre *tre; 
    const char *error; int error_offset;
    
    if((tre = pcre_compile("([a-z0-9_\\-\\.]*?)([a-z0-9_\\-]*?)\\.?([a-z0-9_\\-]+)\\.([a-z0-9_\\-]+)$",0,&error,&error_offset,NULL)) == NULL){
       fprintf(stderr,"[pcre] Error\n"); exit(EXIT_FAILURE); 
    }
    return tre;
}
    
void read_list(kdomain *root,char *bl_filename,int type,pcre *re){
	
    FILE *fp; char line[80];
	if((fp = fopen(bl_filename,"r")) != NULL){
		while(fgets(line,sizeof(line),fp) != NULL){
			if(isalpha(line[0]) || isdigit(line[0])){ 		
            		    if(strchr(line,'.'))
                    		load_domain(line,re,root,type);
			}
		}
	}
	
    fclose(fp);
}


unsigned int hash(const char *str, int len){
    
    unsigned hash = 0;
    int count = 0;
    
    for(count = 0; count < len; count++)
        hash = (int) str[count] + (hash << 6) + (hash << 16) - hash;

    return (hash & 0x7FFFFFFF);
}
