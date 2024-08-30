#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;

/*
 * release memory and other cleanups
 */
void loader_cleanup()
{
  free(ehdr);
  free(phdr);
  ehdr = NULL;
  phdr = NULL;
  close(fd);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe)
{
  fd = open(argv[1], O_RDONLY);
  if (fd == -1)
  {
    printf("Error in opening the file %s", argv[1]);
    exit(1);
  }
  off_t f_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char *heap = (char *)malloc(f_size);
  // 1. Load entire binary content into the memory from the ELF file.
  ssize_t bytes_read = read(fd, heap, f_size);
  if (bytes_read != f_size)
  {
    printf("Error in reading the file %s", argv[1]);
    exit(1);
  }

  // 2. Iterate through the PHDR table and find the section of PT_LOAD
  //    type that contains the address of the entrypoint method in fib.c
  ehdr = (Elf32_Ehdr *)heap;
  Elf32_Off prog_offset = ehdr->e_phoff;
  phdr = (Elf32_Phdr *)heap + prog_offset;
  uint16_t prog_num = ehdr->e_phnum;
  Elf32_Phdr *temp = phdr;
  int i = 0;
  void* virtual_mem;
  while (i < prog_num)
  {
    if (temp->p_type == PT_LOAD)
    {
      // 3. Allocate memory of the size "p_memsz" using mmap function
      //    and then copy the segment content
      virtual_mem = mmap(NULL, temp->p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
     
      
    }
  }

  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n", result);
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Usage: %s <ELF Executable> \n", argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(argv[1]);
  // 3. invoke the cleanup routine inside the loader
  loader_cleanup();
  return 0;
}
