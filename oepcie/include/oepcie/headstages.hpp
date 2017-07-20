#pragma once

#define HEADER_BYTES 12

namespace oe {

// Helpers
struct entry {
  char const *name;
  std::size_t offset;
  std::size_t bytes;
};

constexpr bool same(char const *x, char const *y)
{
    return !*x && !*y ? true : (*x == *y && same(x + 1, y + 1));
}

constexpr size_t offset(char const *name, entry const *entries)
{
    return same(entries->name, name) ? entries->value :
                                       value(name, entries + 1);
}

template <typename Headstage>
constexpr size_t block_bytes()
{
    int end = hs return Headstage::block_layout[end].offset
              + Headstage.block_layout[end].bytes
              + headstage_base::header_bytes();
}

// Headstages

struct headstage_base {

    static constexpr int magic = 0x01;
    static constexpr std::size_t header_bytes()
    {
        return sizeof(int)       // magic 
             + sizeof(uint64_t)  // sample count 
             + sizeof(uint16_t); // data byte count
    }
};

struct mitserdes_128 : public headstage_base {

    static constexpr int id = 1;

    static constexpr entry block_layout[] = {
      { "Neural data", 0, 256 },
      { "IMU data", 256, 18},
      { "Lighthouse data", 274, 18},
    };
};

struct mitserdes_256 : public headstage_base {

    static constexpr int id = 2;

    static constexpr entry block_layout[] = {
      { "Neural data", 0, 512 },
      { "IMU data", 512, 18},
      { "Lighthouse data", 530, 18},
    };
};

} // namespace oe

