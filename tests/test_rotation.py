import filecmp

from nest2D import Point, Box, Item, nest, SVGWriter
from examples.simple_sample import add_shape1, add_shape2
from . import here

def test_rotation():
    output_svg = here('../out.svg')
    expected_svg = here('resources/expected_output_rotation.svg')

    box = Box(150000000, 150000000)

    config = {'rotations': [0, 3.141592653589793]}

    input = []
    add_shape1(23, input)
    add_shape2(15, input)

    nbins, pgrp = nest(input, box, dist=0, **config)
    
    print(nbins)
    for p in pgrp:
        print(p.rotation())

    sw = SVGWriter()
    sw.write_packgroup(pgrp)
    sw.save()

    assert filecmp.cmp(output_svg, expected_svg), 'svg output not as expected!'