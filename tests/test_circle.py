from nest2D import Point, Circle


def test_circle():
    c = Circle(Point(150000000, 150000000), 150000000)
    #assert repr(c) == ''  # TODO
    assert Circle.__doc__ == '2D Circle point pair'
