#include <stdio.h>
#include <string.h>

/* This is the pagemap, as described in the kernel documentation:
 * Bits 0-54  page frame number (PFN) if present
 * Bits 0-4   swap type if swapped
 * Bits 5-54  swap offset if swapped
 * Bits 55-60 page shift (page size = 1<<page shift)
 * Bit  61    reserved for future use
 * Bit  62    page swapped
 * Bit  63    page present
 */
struct __attribute__ ((__packed__)) pagemap_entry {
  unsigned long long pfn:55;
  unsigned long long page_shift:6;
  unsigned long long reserved:1;
  unsigned long long swapped:1;
  unsigned long long present:1;
} pagemap_entry;

/* Specify a process PID on the cmdline.  This program will dump the pagemap
 * to the console.
 */
int main(int argc, char **argv)
{
  char *pid = argv[1];
  char name[256];
  snprintf(name, sizeof(name), "/proc/%s/pagemap", pid);
  printf("name is %s.\n", name);

  struct pagemap_entry entry;
  
  FILE *fp = fopen(name, "r");
  if(fp != (FILE *) 0) {
    int i;
    for(i = 0; i < 20000000; i++) {
      size_t nmemb = fread(&entry, sizeof(entry), 1, fp);
      if(nmemb == 1) {
#ifdef DEBUG
        unsigned char *p = (unsigned char *) &entry;
        int x;
        printf("sizeof(entry) = %d.\n", sizeof(entry));
        printf("Entry:");
        for(x = 0; x < sizeof(entry); x++) {
          printf(" %02x", p[x]);
        }
        printf("\n");
        unsigned long long *ul = (unsigned long long *) &entry;
        printf("Entry: %llu.\n", *ul);
        printf("Entry: %llx.\n", *ul);
#endif

        printf("present    = %d.\n", entry.present);
        printf("swapped:   = %d.\n", entry.swapped);
        printf("reserved   = %d.\n", entry.reserved);
        printf("page_shift = %d.\n", entry.page_shift);
        if(entry.present == 1) {
          printf("pfn        = %lld.\n", entry.pfn);
        }
        else {
          int swap_type = entry.pfn & 0x0000001F;
          unsigned long long swap_offset = entry.pfn >> 5;
            printf("swap_type = %d.\n", swap_type);
            printf("swap_offset = %lld.\n", swap_offset);
        }
      }
      else {
        break;
      }
    }

    fclose(fp);
  }

  return 0;
}

