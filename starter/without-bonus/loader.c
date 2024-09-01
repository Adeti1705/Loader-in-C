#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup()
{
  // no need to free the pointer as it just pointing to null will be enough
  ehdr = NULL;
  phdr = NULL;
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe)
{
  fd = open(*exe, O_RDONLY);
  if (fd == -1)
  {
    printf("Error in opening the file");
    exit(1);
  }
  off_t f_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *heap = (char *)malloc(f_size);
  // 1. Load entire binary content into the memory from the ELF file.
  ssize_t bytes_read = read(fd, heap, f_size);
  if (bytes_read != f_size)
  {
    printf("Error in reading the file");
    exit(1);
  }

  // 2. Iterate through the PHDR table and find the section of PT_LOAD
  //    type that contains the address of the entrypoint method in fib.c
  ehdr = (Elf32_Ehdr *)heap;
  Elf32_Off prog_offset = ehdr->e_phoff;
  phdr = (Elf32_Phdr *)(heap + prog_offset);
  uint16_t prog_num = ehdr->e_phnum;
  unsigned int entrypoint = ehdr->e_entry;
  Elf32_Phdr *temp = phdr;
  int i = 0;
  void *entry = NULL;
  void *virtual_mem = NULL;

  while (i < prog_num)
  {
    if (temp->p_type == PT_LOAD)
    {
      // 3. Allocate memory of the size "p_memsz" using mmap function
      //    and then copy the segment content
      virtual_mem = mmap(NULL, temp->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
      if (virtual_mem == MAP_FAILED)
      {
        perror("Error: Memory mapping failed");
        exit(1);
      }
      memcpy(virtual_mem, heap, temp->p_offset, temp->p_memsz);
      // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
      entry = virtual_mem + (entrypoint - temp->p_vaddr);
      if (entry > virtual_mem && entry <= virtual_mem + temp->p_memsz)
      {
        break;
      }
    }
    i++;
    temp++;
  }
  if (entry)
  {
    // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
    int (*_start)(void) = (int (*)(void))entry;

    // 6. Call the "_start" method and print the value returned from the "_start"
    int result = _start();
    printf("User _start return value = %d\n", result);
  }
  else
  {
    printf("Entrypoint not found in the PT_LOAD segment\n");
    free(heap);
    exit(1);
  }
  close(fd);
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  FILE *elfFile = fopen(argv[1], "rb");
  if (!elfFile)
  {
    printf("Error: Unable to open ELF file.\n");
    exit(1);
  }
  fclose(elfFile);
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv[1]);
  // 3. invoke the cleanup routine inside the loader
  loader_cleanup();
  return 0;
}
