/*
 * VGA graphics driver
 */

#include <stdint.h>

#include <kernel/interrupts.h>
#include <drivers/vga.h>
#include <libc/string.h>

#define VGA_ADDRESS 0xA0000

uint8_t *vga_ptr = (uint8_t *)VGA_ADDRESS;

uint16_t vga_x = 0;
uint16_t vga_y = 0;

void vga_init()
{
	vga_x = 0;
	vga_y = 0;
	memset(vga_ptr, 0, 800*600); /* clear the screen */
}

/*
 * vga_set_pixel - Sets a pixel to a specific colour
 * @x: X co-ordinate of the pixel
 * @y: Y co-ordinate of the pixel
 * @colour: Colour to set the pixel to
 */
void vga_set_pixel(uint16_t x, uint16_t y, uint8_t colour)
{
	if (x >= 800 || y >= 600)
		return;
	uint32_t offset = x + y * 800;
	vga_ptr[offset] = colour;
}

uint8_t vga_get_pixel(uint16_t x, uint16_t y)
{
	if (x >= 800 || y >= 600)
		return 0;
	uint32_t offset = x + y * 800;
	return vga_ptr[offset];
}

/*
 * vga_draw_rect - Draws a rectangle
 * @x: X co-ordinate of the rectangle
 * @y: Y co-ordinate of the rectangle
 * @width: Width of the rectangle
 * @height: Height of the rectangle
 * @colour: Colour to set the pixel to
 */
void vga_draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t colour)
{
	for (uint16_t i = y; i < y + height; i++)
	{
		for (uint16_t j = x; j < x + width; j++)
		{
			vga_set_pixel(j, i, colour);
		}
	}
}
