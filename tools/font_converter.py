import tools_core
import sys
from os import path

#Options
font_struct_template = ""

#Font Variables
fnt_name = ""
fnt_size = 0
fnt_line_height = 0
fnt_pad_left = 0
fnt_pad_right = 0
fnt_pad_top = 0
fnt_pad_bottom = 0
fnt_img_path = ""
fnt_num_glyphs = 0
fnt_img_name = ""

fnt_stream = ""

header_file = None

def load_options():
    global font_struct_template
    options = tools_core.get_tool_options(__file__)

    for line in options["font_struct_template"]:
        font_struct_template += line + "\n"

def load_font_data():
    global fnt_name, fnt_size, fnt_line_height
    global fnt_pad_left, fnt_pad_right, fnt_pad_top
    global fnt_pad_bottom, fnt_img_path, fnt_num_glyphs
    global fnt_stream

    fnt_file_path = ""
    try:
        fnt_file_path = sys.argv[1]
    except IndexError:
        print("usage: <font_file>")
        exit()

    if not path.exists(fnt_file_path):
        print("File\"%\" does not exist" % (fnt_file_path))
        exit()

    fnt_data = ""
    with open(fnt_file_path, "r") as fnt_file:
        fnt_data = fnt_file.read()
        fnt_data = fnt_data.replace("\n", " ").replace("=", " ").replace("\"", "").replace(",", " ")
        fnt_data = fnt_data.split(" ")
        fnt_stream = iter(fnt_data)
        fnt_file.close()
    
    for token in fnt_stream:
        if token == "face":
            fnt_name = next(fnt_stream)
        elif token == "size":
            fnt_size = next(fnt_stream)
        elif token == "padding":
            fnt_pad_top = int(next(fnt_stream))
            fnt_pad_right = int(next(fnt_stream))
            fnt_pad_bottom = int(next(fnt_stream))
            fnt_pad_left = int(next(fnt_stream))
        elif token == "lineHeight":
            fnt_line_height = int(next(fnt_stream))
        elif token == "file":
            fnt_img_path = next(fnt_stream)
        elif token == "chars" and next(fnt_stream) == "count":
            fnt_num_glyphs = int(next(fnt_stream))
            break 

def create_header_file():
    global header_file

    header_file = open("%s_font.hpp" % fnt_name, "w")

def write_image_data():
    global fnt_img_name
    global header_file

    fnt_img_name = fnt_img_path.split(".")[0]

    img_file_bytes = bytearray()
    with open(fnt_img_path, "rb") as f:
        img_file_bytes = bytearray(f.read())

    header_file.write("const unsigned char %s_png[] =\n{\n" % (fnt_img_name))
    
    bytes_per_line = 20
    bytes_count = 0
    for i, byte in enumerate(img_file_bytes):
        num_format = "0x{:02X}, "
        if i == len(img_file_bytes) - 1:
            num_format = "0x{:02X}"

        header_file.write(num_format.format(byte))

        if bytes_count == bytes_per_line - 1:
            header_file.write("\n")

        bytes_count += 1
        bytes_count %= bytes_per_line

    if bytes_count != 0:
        header_file.write("\n")

    header_file.write("};\n\n")

    print("Wrote \"%s\" texture." % (fnt_img_path))

class Glyph:
    id = 0
    x = 0
    y = 0
    width = 0
    height = 0
    xoffset = 0
    yoffset = 0
    xadvance = 0

def write_glyph_data():
    glyphs = []

    for token in fnt_stream:
        if token == "char" and next(fnt_stream) == "id":
            glyphs.append(Glyph())
            glyphs[-1].id = int(next(fnt_stream))
        elif token == "x":
            glyphs[-1].x = int(next(fnt_stream))
        elif token == "y":
            glyphs[-1].y = int(next(fnt_stream))
        elif token == "width":
            glyphs[-1].width = int(next(fnt_stream))
        elif token == "height":
            glyphs[-1].height = int(next(fnt_stream))
        elif token == "xoffset":
            glyphs[-1].xoffset = int(next(fnt_stream))
        elif token == "yoffset":
            glyphs[-1].yoffset = int(next(fnt_stream))
        elif token == "xadvance":
            glyphs[-1].xadvance = int(next(fnt_stream))

    header_file.write("const EMFontRenderer::Glyph %s_glyphData[] =\n{\n" % fnt_img_name)
    
    glyph_template = "    {{{0:4d}, {1:4d}, {2:4d}, {3:4d}, {4:4d}, {5:4d}, {6:4d}}},\n"
    for i in range(255):
        glyph = Glyph()
        for g in glyphs:
            if g.id == i:
                glyph = g

        header_file.write(glyph_template.format(
            glyph.x,
            glyph.y,
            glyph.width,
            glyph.height,
            glyph.xoffset,
            glyph.yoffset,
            glyph.xadvance
        ))

    header_file.write("};\n\n")

    print("Wrote glyph data.")

def write_font_struct():
    header_file.write(font_struct_template.format(
        name = fnt_name,
        font_height = fnt_size,
        line_height = fnt_line_height,
        left_pad = fnt_pad_left,
        right_pad = fnt_pad_right,
        top_pad = fnt_pad_top,
        bottom_pad = fnt_pad_bottom,
        glyphs_ptr = fnt_img_name + "_glyphData",
        tex_png = fnt_img_name + "_png",
        num_glyphs = fnt_num_glyphs
    ))

    print("Wrote font data")

def main():
    if tools_core.need_help(sys.argv):
        print(tools_core.get_tool_help(__file__))
        return
    
    load_options()
    load_font_data()
    create_header_file()
    write_image_data()
    write_glyph_data()
    write_font_struct()

    print("Successfully created font header file")
    header_file.close()

if __name__ == "__main__":
    main()