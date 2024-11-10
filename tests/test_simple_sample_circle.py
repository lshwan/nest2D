import filecmp

from nest2D import Point, Circle, Item, nest, SVGWriter
from examples.simple_sample import add_shape1, add_shape2
from . import here

def test_simple_sample_circle():
    output_svg = here('../out.svg')
    expected_svg = here('resources/expected_output_circle.svg')

    circle = Circle(Point(120000000, 120000000), 120000000)

    input = []
    add_shape1(23, input)
    add_shape2(15, input)

    nbins, pgrp = nest(input, circle)
    
    sw = SVGWriter()
    sw.write_packgroup(pgrp)
    sw.save()

    assert filecmp.cmp(output_svg, expected_svg), 'svg output not as expected!'