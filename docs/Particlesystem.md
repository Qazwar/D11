# Spawner

The spawner defines the number of particles to emitt and the texture to use.

```spawn { 
	texture : 80,200,20,20
    duration : 0
    rate : 32
    loop : 0
    loop_delay : 0.0
}```

# Modules

Every module can be used to generate and update the particles

## Ring emitter

| Parameter      | Description               |
| -------------- | ------------------------- |
| radius         | radius of the ring        |
| variance       | variance of the radius    |
| angle_variance | angle variance            |
| step           | the angle step (optional) |

Example:

```ring_location {
    radius : 40
    variance : 4
    angle_variance : 10
    step : 0
}```

## Lifetime

| Parameter | Description                   |
| --------- | ----------------------------- |
| ttl       | time to live of the particle  |
| variance  | variance of ttl               |

Example:

```lifecycle { 
    ttl : 1.0
    variance : 0.2
}```

## Color

```color {
	hsv : 0,31,100
    alpha : 255
    hue_variance : 0
    saturation_variance : 10
    value_variance : 10
	start : 255,204,68,255
	end : 255,204,68,32
}```

## Size

The size module supports two different kind of updates. First you can define a min and max value 
and the size will be scaled linear. The actual value is always multiplied by the initial size so
these values are taken as a percentage. The other way is using a path.

| Parameter | Description                               |
| --------- | ----------------------------------------- |
| initial   | the initial size of the particle          |
| variance  | variance of the initial size              |
| path      | a v2 path describing the size             |
| min       | the min value multiplied by initial size  |
| max       | the max value                             |
          
Example linear update:
            
```size { 
	initial : 2.0,0.4
	variance : 0.0,0.0
	min : 0.8,0.8
    max : 1.1,0.4
}```

Example path update:
            
```size { 
	initial : 2.0,0.4
	variance : 0.0,0.0
	path: 0.0,0.8,0.8,0.7,0.7,0.7,1.0,0.4,0.4
}```

## Alpha

| Parameter | Description                               |
| --------- | ----------------------------------------- |
| initial   | the initial alpha value of the particle   |
| variance  | variance of the initial size              |
| path      | a float path describing the size          |
| min       | the min value multiplied by initial alpha |
| max       | the max value                             |

Example using path:

```alpha { 
	initial : 1.0
	variance : 0.0
	path: 0.0,0.8,0.7,0.7,1.0,0.4
}```

Example using linear updates:

```alpha { 
	initial : 1.0
	variance : 0.0
	min : 1.0
    max : 0.1
}```

## Rotation

| Parameter       | Description                                  |
| --------------- | -------------------------------------------- |
| velocity        | the rotation velocity in degrees             |
| variance        | variance of the roation velocity in degress  |
| velocity_range  | the rotation velocity range in degrees       |

If you use velocity and range it will be converted to velocity_range internally.

Example using velocity and range:

```rotation {
    velocity : 360
    variance : 20
}```

Example using velocityrange:

```rotation {
    velocity_range : 360,720
}```