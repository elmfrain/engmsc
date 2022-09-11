This directory contains Python3 scripts that can be utilized to aid with development.

This readme file contains all the help information for each script. You may refer to
this file for help information or pass the -h, --help flag when executing a script to
print out help information for that script.

If adding help information for a new script, you must use the following format:

  Script: <script_name>
    <help_info>
  EndScript

or else the -h, --help functionality for that script may not work.

Script: font_converter.py
  Usage: <font_file>

  Description: Generates a font header file for it to be used by the application.
  The application currently only accepts header files for importing fonts into it,
  and it contains glyphs and information to render a font. The input file must be
  a .fnt file generated with Hiero (https://libgdx.com/wiki/tools/hiero) along
  with its .png file.

  Example: python3 font_converter.py consola.fnt
EndScript