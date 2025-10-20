import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure()

Size1 = 8*8
Size2 = 8*8#8*5

phi = np.linspace((22.5*np.pi)/180, (180 - 22.5*np.pi)/180, Size2)
phi = np.linspace(np.pi/8, 7*np.pi/8, Size2)
theta = np.linspace(0, np.pi, Size1)


z = np.zeros((Size1, Size2))
x = np.zeros((Size1, Size2))
y = np.zeros((Size1, Size2))

r = np.ones((Size1, Size2))*20

for t in range(len(theta)):
    for p in range(len(phi)):
        x[t][p] = r[t][p]*np.cos(theta[t])*np.cos(phi[p])
        y[t][p] = r[t][p]*np.sin(theta[t])*np.cos(phi[p])
        z[t][p] = r[t][p]*np.sin(phi[p])

ax = fig.add_subplot(111, projection='3d')
ax.set_aspect("equal")
ax.plot_surface(x, y, z)
plt.show()