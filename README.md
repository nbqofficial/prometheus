# Prometheus - Hacking framework

### Why?
This is an old project of mine that I no longer find useful, but I believe it could be useful to some of you and you might find great value in this. The project is outdated for quite some time, but updating it should be trivial for anyone even remotely proficient.

### Overview
Prometheus is a non-specific hacking framework and it could have many use cases in different domains. The way it works from a high-level perspective is by hooking the dispatch handler of legit Windows driver called spaceport.sys. 
And it does this in a specific way, by first scanning the spaceport .text section for contiguous memory of 12 INT3 bytes (0xCC). 
The reason for this is because the OS wont overwrite this memory which might happen if it was zeroed out and the hook we place is 12 bytes in size. The manual mapped driver writes the shellcode in this 0xCC codecave inside spaceport and after saving the original dispatch handler, it replaces the pointer to point to the code cave with our shellcode hook.
Hook job is to detour all dispatch calls to spaceport to our manual mapped driver dispatch so that we can use it as proxy for km-um communication and also retain the original capability of spaceport. Also anyone looking at spaceport dispatch address will determine that it belongs to spaceport address space and thus is "okay" *wink wink*.
Communication is as easy as using core dll interface to talk to our mapped driver using spaceport as middleman. Everything is explained in template solution.

### What can it do?
The purpose of this framework was to provide you with a tool that can be used to hack or manipulate any process on the system. It has capabilities to obtain process information, read and write memory, get module base addresses, simulate mouse movement, draw on screen etc.
It is meant to be a one-stop-shop for anyone looking into how to develop this kind of framework.

### Enjoy :)
