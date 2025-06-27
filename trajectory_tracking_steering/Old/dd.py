import numpy as np

# 각 물체의 위치 (x, y)와 질량
positions = np.array([
    [0.0, 0.177],
    [0.70836, 0.181],
    [0.06205, 0.364655],
    [0.4, 0.36],
    [0.47387, 0.7]
])

masses = np.array([
    2.6538,
    4.487497,
    2.653763,
    8.522351,
    1.43837
])

# 전체 질량
total_mass = np.sum(masses)

# 질량 중심 계산
center_of_mass = np.sum(positions.T * masses, axis=1) / total_mass

print(f"Center of Mass (x, y): ({center_of_mass[0]:.4f}, {center_of_mass[1]:.4f})")