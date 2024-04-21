
from math import pi, sin, cos, acos
from dataclasses import dataclass
from sklearn.neighbors import BallTree
@dataclass
class Point:
    az: float
    elev: float

    def __init__(self, az, elev) -> None:
        self.az = az/360*2*pi
        self.elev = elev/360*2*pi

    def dist(self, other):
        d_lambda = other.az-self.az
        inner = sin(self.elev) * sin(other.elev) + cos(self.elev) * cos(other.elev) * cos(d_lambda)
        d_rho = acos(inner)
        return d_rho


points = [Point(0, 0), Point(90, 0), Point( 180, 0),   Point( 270, 0), Point(0, 90 )]

bt = BallTree([(p.elev, p.az) for p in points])
dist, indices = bt.query([(0, 0)], 2)
print(dist, indices)
for p in points:
    # print(p.az, p.elev)
    print(points[0].dist(p))
