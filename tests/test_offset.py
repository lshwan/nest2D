import filecmp

from nest2D import Point, Box, Item, nest, SVGWriter
from examples.simple_sample import add_shape1, add_shape2
from . import here

def test_offset():
    output_svg = here('../out.svg')
    expected_svg = here('resources/expected_output_offset.svg')

    box = Box(180000000, 180000000)
    dist = 4500000

    input = []
    add_shape1(23, input)
    add_shape2(15, input)

    nbins, pgrp = nest(input, box, dist=dist)
    
    sw = SVGWriter()
    sw.write_packgroup(pgrp)
    sw.save()

    assert filecmp.cmp(output_svg, expected_svg), 'svg output not as expected!'