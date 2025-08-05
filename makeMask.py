bit_map = {
    "c1a": "000001010001",
    "c2a": "000011010011",
    "c3a": "000101010101",
    "c4a": "000111010111",
    "c5a": "001001011001",
    "c6a": "001011011011",
    "e1a": "000010",
    "e2a": "000100",
    "e3a": "000110",
    "e4a": "001000",
    "e5a": "001010",
    "e6a": "001100",

    "c1b": "100001110001",
    "c2b": "100011110011",
    "c3b": "100101110101",
    "c4b": "100111110111",
    "c5b": "101001111001",
    "c6b": "101011111011",
    "e1b": "100010",
    "e2b": "100100",
    "e3b": "100110",
    "e4b": "101000",
    "e5b": "101010",
    "e6b": "101100"
}

def build_parts(input_str):
    tokens = input_str.split()
    bit_string = ''
    mask_string = ''
    for token in tokens:
        if token == "x":
            bit_string += "000000"
            mask_string += "000000"
        elif token == "xx":
            bit_string += "000000000000"
            mask_string += "000000000000"
        else:
            bit_string += bit_map[token]
            mask_string += "1" * len(bit_map[token])

    bit_string = '0' * 20 + bit_string
    mask_string = '0' * 20 + mask_string

    bit_hex = hex(int(bit_string, 2))[2:].zfill((len(bit_string) + 3) // 4)
    mask_hex = hex(int(mask_string, 2))[2:].zfill((len(mask_string) + 3) // 4)

    mid = len(bit_hex) // 2
    bit_output = f"static_cast<Puzzle::Row>(0x{bit_hex[:mid]}ULL) << 64 | 0x{bit_hex[mid:]}ULL,"

    mid = len(mask_hex) // 2
    mask_output = f"static_cast<Puzzle::Row>(0x{mask_hex[:mid]}ULL) << 64 | 0x{mask_hex[mid:]}ULL,"

    return bit_output, mask_output

# SOLVED TOP: "c1a e1a c2a e2a c3a e3a c4a e4a c5a e5a c6a e6a"
# SOLVED BOTTOM: "e3b c3b e2b c2b e1b c1b e6b c6b e5b c5b e4b c4b"

top_bit, top_mask = build_parts("c1a e1a xx e2a xx e3a xx e4a xx e5a xx e6a")
bottom_bit, bottom_mask = build_parts("e3b c3b e2b c2b x xx e6b c6b x xx e4b c4b")

print(top_bit)
print(top_mask)
print(bottom_bit)
print(bottom_mask)