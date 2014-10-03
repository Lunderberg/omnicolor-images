#!/usr/bin/env python3

from PIL import Image
import random

def poprandom(things):
    output = random.sample(things,1)[0]
    things.remove(output)
    return output

def default():
    im = Image.new('RGB',(256,128))
    allowed_colors = set((i<<3 & 0xF8, i>>2 & 0xF8, i>>7 & 0xF8)
                         for i in range(32768))
    return im,allowed_colors

def ordered():
    im,cols = default()
    for i,col in enumerate(sorted(list(cols))):
        x = i % 256
        y = i // 256
        im.putpixel((x,y),col)
    im.show()

def growth():
    im,cols = default()
    cols = sorted(list(cols))
    unexplored = set((x,y) for x in range(256) for y in range(128))
    frontier = set([poprandom(unexplored)])
    body = {}

    while frontier:
        loc = poprandom(frontier)
        col = cols.pop()
        body[loc] = col
        x,y = loc
        for dx in [-1,0,1]:
            for dy in [-1,0,1]:
                new_loc = (x+dx,y+dy)
                if new_loc in unexplored:
                    unexplored.remove(new_loc)
                    frontier.add(new_loc)
        im.putpixel(loc,col)

    im.show()

def closest_growth():
    im,cols = default()
    cols = sorted(list(cols))
    unexplored = set((x,y) for x in range(256) for y in range(128))
    frontier = set([poprandom(unexplored)])
    body = {}

    while frontier:
        print('Body: {}\tFrontier: {}\tUnexplored: {}'.format(len(body),len(frontier),len(unexplored)))

        loc = poprandom(frontier)
        x,y = loc

        # Find all neighboring colors
        neighbor_colors = []
        for dx in [-1,0,1]:
            for dy in [-1,0,1]:
                neighbor = (x+dx,y+dy)
                if neighbor in body:
                    neighbor_colors.append(body[neighbor])

        # Find the closest color to the neighbors
        if neighbor_colors:
            average_r = sum(c[0] for c in neighbor_colors)/len(neighbor_colors)
            average_g = sum(c[1] for c in neighbor_colors)/len(neighbor_colors)
            average_b = sum(c[2] for c in neighbor_colors)/len(neighbor_colors)
            col = min(cols,key=lambda c:(c[0]-average_r)**2 + (c[1]-average_g)**2 + (c[2]-average_b)**2)
            cols.remove(col)
        else:
            col = poprandom(cols)

        body[loc] = col
        for dx in [-1,0,1]:
            for dy in [-1,0,1]:
                new_loc = (x+dx,y+dy)
                if new_loc in unexplored:
                    unexplored.remove(new_loc)
                    frontier.add(new_loc)
        im.putpixel(loc,col)

    im.show()

print('Ordered')
ordered()
print('Growth')
growth()
print('Closest Growth')
closest_growth()
