void
memcpy (void *destination, void *source, int length)
{
  /* naive implementation */
  int i;

  for (i = 0; i < length; i++)
    *(char *)destination++ = *(char *)source++;
}
