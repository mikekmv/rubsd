/*

NUMA - non-uniform memory access.  Access to local memory is fast,
remote memory is slow.

How does this apply to a386?  Well, several a386's could be running on
different network nodes, communicating with each other to give the
impression of one big machine.

Of course, access to memory on another network node will be slow,
especially access to shared writable pages.  But then, that's normal
on NUMA machines.

Memory type:                    Access method:
local read-only                 direct access
local read-write, COW           direct access
local read-write, shared        direct access (unless being written remotely)
remote read-only                copied from remote node, then direct access
remote read-write, COW          copied from remote node, then direct access
remote read-write, shared       something painful

Some pseudo-code:

map_remote_memory ()
{
  get_physical_memory_map (remote_node);
  for each page
    {
      copy_page_via_network ();
      switch (page_type)
        {
	case READ_ONLY:
          set_read_only ();
          break;
        case READ_WRITE_PRIVATE:
        case READ_WRITE_COW:
        case READ_WRITE_SHARED:
          set_read_only ();
          break;
        }
    }
}

*/
