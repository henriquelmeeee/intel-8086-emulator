#pragma once

struct vm_state {
  unsigned long vm_memory_size;
  char vm_mode; // 16 bits por padrão
} __attribute__((packed));
