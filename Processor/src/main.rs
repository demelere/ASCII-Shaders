use anyhow::{Context, Result};
use clap::Parser;
use image::{DynamicImage, GenericImageView, ImageBuffer, Rgba};
use rayon::prelude::*;
use serde::{Deserialize, Serialize};
use std::path::{Path, PathBuf};
use walkdir::WalkDir;

mod ascii_converter;
use ascii_converter::*;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Cli {
    /// Input path (file or directory)
    #[arg(short, long)]
    input: Vec<PathBuf>,

    /// Output directory
    #[arg(short, long)]
    output: PathBuf,

    /// Settings file path (JSON)
    #[arg(short, long)]
    settings: PathBuf,
}

#[derive(Debug, Serialize, Deserialize)]
struct AsciiSettings {
    block_size: u32,
    brightness: f32,
    auto_adjust: bool,
    color: bool,
    invert: bool,
    edge_detection: bool,
    sigma1: f32,
    sigma2: f32,
    ascii_chars: String,
    dithering: bool,
    palette: String,
    color_palette: Vec<String>,
    foreground: String,
    background: String,
}

impl Default for AsciiSettings {
    fn default() -> Self {
        Self {
            block_size: 8,
            brightness: 1.0,
            auto_adjust: true,
            color: true,
            invert: false,
            edge_detection: false,
            sigma1: 2.0,
            sigma2: 2.0,
            ascii_chars: " .:-=+*#%@".to_string(),
            dithering: false,
            palette: "original".to_string(),
            color_palette: vec!["#ffffff".to_string()],
            foreground: "#000000".to_string(),
            background: "#ffffff".to_string(),
        }
    }
}

struct ImageProcessor {
    settings: AsciiSettings,
}

impl ImageProcessor {
    fn new(settings: AsciiSettings) -> Self {
        Self { settings }
    }

    fn process_image(&self, input_path: &Path, output_path: &Path) -> Result<()> {
        let img = image::open(input_path)
            .with_context(|| format!("Failed to open image: {}", input_path.display()))?;
        
        let processed = if self.settings.edge_detection {
            self.process_with_edges(&img)
        } else {
            self.process_normal(&img)
        };

        processed.save(output_path)
            .with_context(|| format!("Failed to save image: {}", output_path.display()))?;
        
        Ok(())
    }

    fn process_normal(&self, img: &DynamicImage) -> DynamicImage {
        let mut processed = to_grayscale(img);
        
        if self.settings.auto_adjust {
            auto_adjust_brightness(&mut processed);
        }
        
        adjust_brightness(&mut processed, self.settings.brightness);
        
        if self.settings.dithering {
            apply_dithering(&mut processed);
        }
        
        self.create_ascii_art(&processed)
    }

    fn process_with_edges(&self, img: &DynamicImage) -> DynamicImage {
        let processed = detect_edges(img, self.settings.sigma1, self.settings.sigma2);
        self.create_ascii_art(&processed)
    }

    fn create_ascii_art(&self, img: &ImageBuffer<Rgba<u8>, Vec<u8>>) -> DynamicImage {
        let (width, height) = img.dimensions();
        let block_size = self.settings.block_size;
        let new_width = width / block_size;
        let new_height = height / block_size;
        
        let mut output = ImageBuffer::new(width, height);
        
        for y in 0..new_height {
            for x in 0..new_width {
                // Calculate average brightness for the block
                let mut sum = 0u32;
                let mut count = 0u32;
                let mut avg_color = [0u32; 3];
                
                for by in 0..block_size {
                    for bx in 0..block_size {
                        let px = x * block_size + bx;
                        let py = y * block_size + by;
                        if px < width && py < height {
                            let pixel = img.get_pixel(px, py);
                            sum += pixel[0] as u32;
                            for c in 0..3 {
                                avg_color[c] += pixel[c] as u32;
                            }
                            count += 1;
                        }
                    }
                }
                
                let avg_brightness = (sum / count) as u8;
                let ascii_char = get_ascii_char(avg_brightness, &self.settings.ascii_chars, self.settings.invert);
                
                // Calculate average original color
                let original_color = [
                    (avg_color[0] / count) as u8,
                    (avg_color[1] / count) as u8,
                    (avg_color[2] / count) as u8,
                ];
                
                // Draw the ASCII character
                for by in 0..block_size {
                    for bx in 0..block_size {
                        let px = x * block_size + bx;
                        let py = y * block_size + by;
                        if px < width && py < height {
                            let color = if self.settings.color {
                                let rgb = get_color_for_brightness(avg_brightness, &self.settings.palette, original_color);
                                Rgba([rgb[0], rgb[1], rgb[2], 255])
                            } else {
                                let v = if ascii_char == ' ' { 255 } else { 0 };
                                Rgba([v, v, v, 255])
                            };
                            output.put_pixel(px, py, color);
                        }
                    }
                }
            }
        }
        
        DynamicImage::ImageRgba8(output)
    }
}

fn main() -> Result<()> {
    let cli = Cli::parse();
    
    // Load settings from JSON file
    let settings: AsciiSettings = if cli.settings.exists() {
        let settings_str = std::fs::read_to_string(&cli.settings)
            .context("Failed to read settings file")?;
        serde_json::from_str(&settings_str)
            .context("Failed to parse settings JSON")?
    } else {
        AsciiSettings::default()
    };

    // Create output directory if it doesn't exist
    std::fs::create_dir_all(&cli.output)
        .context("Failed to create output directory")?;

    let processor = ImageProcessor::new(settings);

    // Process all input paths
    for input_path in &cli.input {
        if input_path.is_file() {
            let output_path = cli.output.join(
                input_path
                    .file_name()
                    .unwrap()
                    .to_string_lossy()
                    .replace(".", "_ascii.")
            );
            processor.process_image(input_path, &output_path)?;
        } else if input_path.is_dir() {
            // Process all images in directory
            WalkDir::new(input_path)
                .into_iter()
                .filter_map(Result::ok)
                .filter(|e| {
                    e.path()
                        .extension()
                        .map(|ext| {
                            let ext = ext.to_string_lossy().to_lowercase();
                            matches!(ext.as_str(), "jpg" | "jpeg" | "png" | "gif" | "bmp")
                        })
                        .unwrap_or(false)
                })
                .par_bridge() // Enable parallel processing
                .try_for_each(|entry| {
                    let rel_path = entry
                        .path()
                        .strip_prefix(input_path)
                        .unwrap_or(entry.path());
                    let output_path = cli.output.join(
                        rel_path
                            .to_string_lossy()
                            .replace(".", "_ascii.")
                    );
                    
                    if let Some(parent) = output_path.parent() {
                        std::fs::create_dir_all(parent)?;
                    }
                    
                    processor.process_image(entry.path(), &output_path)
                })?;
        }
    }

    Ok(())
}
