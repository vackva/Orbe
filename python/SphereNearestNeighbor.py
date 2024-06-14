from math import pi, sin, cos, acos
from dataclasses import dataclass
import numpy as np
from sklearn.neighbors import BallTree, KDTree
import sofar as sf
import pyfar as pf
from pathlib import Path

sofa_path = Path(__file__).parent.parent / "assets"


@dataclass
class Point:
    az: float
    elev: float

    def __init__(self, az, elev) -> None:
        self.az = az / 360 * 2 * pi
        self.elev = elev / 360 * 2 * pi

    def dist(self, other):
        d_lambda = other.az - self.az
        inner = sin(self.elev) * sin(other.elev) + cos(self.elev) * cos(
            other.elev
        ) * cos(d_lambda)
        d_rho = acos(inner)
        return d_rho

    def as_tuple(self, rev=False):
        if rev:
            return (self.elev, self.az)
        else:
            return (self.az, self.elev)

    def __str__(self) -> str:
        return f"Point(az={round(self.az, 2)}, elev={round(self.elev, 2)})"


def construct_kdtree(points: list[Point], depth: int):
    axis = depth % 3


def find_nearest_naive(point: Point, points: list[Point]):
    distances = [point.dist(p) for p in points]
    idx = np.argmin(distances)
    return distances[idx], idx


sofa = sf.read_sofa(sofa_path / "pp1_HRIRs_measured.sofa")

coords = pf.samplings.sph_lebedev(sh_order=10)

to_find = pf.Coordinates(1, 0, 0)

index, distance = coords.find_nearest(to_find)

points = [Point(x[0], x[1]) for x in sofa.SourcePosition]
# points = [Point(0, 0), Point(90, 0), Point(180, 0), Point(270, 0), Point(0, 90)]

bt = BallTree([p.as_tuple(rev=True) for p in points], metric="haversine")

testpoints = [
    Point(x[0] * 360, x[1] * 180 - 90) for x in np.random.random_sample((200, 2))
]

points_successful = 0
points_failed = 0
for test_point in testpoints:
    d_test, idx_test = bt.query([test_point.as_tuple(rev=True)])
    d_correct, idx_correct = find_nearest_naive(test_point, points)
    if idx_test[0][0] != idx_correct:
        print(
            f"failed for point {test_point}, idx_correct {idx_correct}, idx_test {idx_test[0][0]}, d_test {round(d_test[0], 2)}, d_correct {round(d_correct, 2)}"
        )
        points_failed += 1
    else:
        print(
            f"success for point {test_point}, idx {idx_correct}, d_test {round(d_test[0][0], 2)}, d_correct {round(d_correct, 2)}"
        )
        points_successful += 1

print(f"\n successful: {points_successful}, failed: {points_failed}")
