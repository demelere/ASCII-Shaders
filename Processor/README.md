# ASCII Art Image Processor

A Rust program that converts images to ASCII art, with support for processing multiple images and directories.

## Features

- Process single images or entire directories
- Parallel processing for faster conversion
- Customizable settings via JSON configuration
- Support for edge detection
- Multiple color palette options
- Brightness adjustment and auto-contrast
- Floyd-Steinberg dithering
- Custom ASCII character sets
- Invert option for dark/light modes

## Installation

1. Make sure you have Rust installed on your system. If not, install it from [rustup.rs](https://rustup.rs/)
2. Clone this repository
3. Build the project:
```bash
cargo build --release
```

## Usage

The program can be run with the following command-line arguments:

```bash
./target/release/ascii_art_processor -i <input_path> -o <output_directory> -s <settings_file>
```

### Arguments

- `-i, --input`: Input path(s). Can be files or directories. Multiple inputs are supported.
- `-o, --output`: Output directory where processed images will be saved
- `-s, --settings`: Path to the JSON settings file (optional, will use defaults if not provided)

### Examples

Process a single image:
```bash
./target/release/ascii_art_processor -i image.jpg -o output/
```

Process multiple images:
```bash
./target/release/ascii_art_processor -i image1.jpg -i image2.png -o output/
```

Process an entire directory:
```bash
./target/release/ascii_art_processor -i images_directory/ -o output/
```

Process with custom settings:
```bash
./target/release/ascii_art_processor -i images_directory/ -o output/ -s settings.json
```

## Settings

The program uses a JSON configuration file to customize the ASCII art conversion. You can find a template in `settings_template.json`. Here are the available settings:

- `block_size`: Size of each ASCII character block (default: 8)
- `brightness`: Brightness adjustment factor (default: 1.0)
- `auto_adjust`: Enable automatic contrast adjustment (default: true)
- `color`: Preserve original colors when using "original" palette (default: true)
- `invert`: Invert brightness values (default: false)
- `edge_detection`: Use edge detection mode (default: false)
- `sigma1`: Edge detection strength parameter 1 (default: 2.0)
- `sigma2`: Edge detection strength parameter 2 (default: 2.0)
- `ascii_chars`: String of ASCII characters to use (default: " .:-=+*#%@")
- `dithering`: Enable Floyd-Steinberg dithering (default: false)
- `palette`: Color palette to use (default: "original")
- `foreground`: Foreground color in hex (default: "#000000")
- `background`: Background color in hex (default: "#ffffff")

### Available Color Palettes

The program supports several color palettes through the `palette` setting:

1. `"original"`: Uses the original colors from the input image
2. `"black_white"`: Simple black and white output
3. `"terminal"`: Classic 16-color terminal palette
4. `"amber"`: Vintage amber monitor look (black and amber)
5. `"low_contrast"`: Subtle grayscale variations
6. `"nord"`: Nord color theme with cool, arctic colors
7. `"catppuccin"`: Catppuccin theme with soft, pastel colors

To use original colors from the input image:
```json
{
  "palette": "original",
  "color": true
}
```

For any other palette, simply set the palette name:
```json
{
  "palette": "nord"
}
```

## Supported Image Formats

- JPEG/JPG
- PNG
- GIF
- BMP

## License

MIT License 