## liboepcie Implementation TODO

- [ ] When we are writing/reading from streams, there are a lot of read/write
  size arguments that are specified as sizeof(int). But things like int are
  implementation specific and don't guarantee a particular byte count. We need
  to make these exact.
- [ ] Signal stream is needed for configuration setting/getting acknowledgements
- [ ] Replace sequential calls to write() in `oe_write_reg` and `oe_read_reg` with
  explicit lseek calls to correct registers followed by write. We might need to
  include these offsets in the header stream? Or just hardcode them.
