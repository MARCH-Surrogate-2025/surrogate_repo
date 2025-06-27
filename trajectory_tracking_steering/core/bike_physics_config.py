import math as m

dt = 1 / 60

a = 0.3763
b = 0.70836
c = 0.0105
h = 0.3201
HeadTubeAngle = 76.5
sinHeadTubeAngle = m.sin(m.radians(HeadTubeAngle))

v_x = (a * c * 9.81 * sinHeadTubeAngle / h) ** 0.5
v_x *= 3

K0 = -(h * v_x ** 2 * sinHeadTubeAngle - a * c * 9.81 * sinHeadTubeAngle ** 2) / (9.81 * b * h)
K2 = 0.7
K3 = 1 / K0
K1 = -K3 * 1.7
K4 = 0.7

M1 = a * h * v_x * sinHeadTubeAngle
M2 = h * v_x ** 2 * sinHeadTubeAngle - a * c * 9.81 * sinHeadTubeAngle ** 2
M3 = b * h ** 2
M4 = b * 9.81 * h
