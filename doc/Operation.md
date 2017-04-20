# Operation and configuration of Victoria to win the RoboMagellan contest.

There are some setup procedures that need to be done and some tools that are available to help.

**INSERT MORE SETUP INFO HERE**

----
# **Configuring the cone detector**
When the **cone_detector.launch** is invoked, there are a couple of tools you can use to control and setup Victoria.

* Running **rqt** and using the dynamic reconfiguration plugin, if you click on the "cone_detector" item in the list on the left, you should see something like

![Dynamic reconfiguration for the cone detector](/doc/rqt_dynamic_configure_cone_detector.png)

There are two cone detector filters with their own set of parameters, and a few parameters that are common to the whole cone detector process.

The two filter parameter names are prefixed with "a" and "b". E.g., alow_hue_, ahigh_hue_. The two filter results are "OR"-ed together. 
You could use the a-filter to pick up the low hue range of the cone and the b-filter to pick up the high hue range, since the orange cone can have a color that spans both ends of the hue scale.
Or, you could use the a-filter to pick up most of the cone and the b-filter to pick up the specular highlights which are out of range of the a-filter, being nearly colorless.

Besides the "a" and "b" filters, after those two filter results are "OR"-ed together, additional parameters apply.

* **min_cone_area_** and **max_cone_area_**

This is a filter to prevent colored balls, leaves, sides of barns and such from being acceptable candidates. 
The cone detector computes a poloygon to encompass each connected region of pixels from the joined filters and considers each of
the resulting polygons a potential cone candidate. One test made is whether the area of that polygon seems reasonable. 
The image was downsampled to 320 by 240, so it's independent of the original picture resolution. 
If the polygon doesn't have an enclosed area in square pixels within the closed interval defined by these two values, the polygon
is rejected as not a cone.

* **max_aspect_ratio_**

The cone detector finds the horizontal centerline of the polynomial enclosing the candidate cone in the image and then finds the leftmost
x coordinate both above and below the centerline, and the rightmost x points above and below. It then compares the length of a line drawn
between the leftmost and rightmost x coordinates above the centerline (width of top of the polynomial) with the length of a line
drawn between the leftmost and rightmost x coordinates below the centerline (width of bottom of polynomial). If the upper line
length is more that **max_aspect_ratio_** times the bottom line length, the polygon is rejected as not pointy enough.

* **poly_epsilon_**

When the cone detector attempts to surround a region of connected pixels with a polygon, it wants to form a simple polygon, ignoring minor
jaggies along the edge. This parameter indicates how many pixels away from an ideal line a point can be and still be considered
on a candidate polygon line. Larger values will result in blobs being represented by polygons of fewer sides, but possibly over
simplifying the polygon. Smaller values will result in minor aberrations in the blob causing a too-complex polygon to be formed.
The resulting polygon is examined for how many sides it used to surround the blob. If the polygon has less than three sides or more than ten,
it is rejected as being too simple or too complex a shape.

* **erode_kernel_size_**

After the two filters are "OR"-ed together, a two step process is taken to try to get rid of noisey pixels. The two steps attempt
to expand black areas (things not considered possible pixels in the cone) to cover over small bits of white areas (things considered
as possible pixels in the cone), and then shrink the white areas (or I've got white and black backwards—doesn't matter). This is done
by forming an "n by n" ellipse, an image processing "kernel" that is moved over every pixel in the image and performing an erode 
and dilate process over the pixels encompassed by the kernel. This value determines the size of that kernel. Large values will
remove useful distinguishing details from the potential pixels in the cone, and small values will leave speckles in the picture.

* **debug_**

If you click on the **debug\_** checkbox, four windows will pop up. You can drag them around and rearrange them, even resize them. An example layout is shown below.

![Cone detector debug windows](/doc/cone_detector_debug_windows.png)

The "A HSV" window shows the output of the threshold operation of the original, downsampled color image after applying the a-prefixed size HSV ranges. When you drag one of the six sliders and release it, the window will show you the resulting thresholded image. You want to come up with a range of values that only shows the cone as much as possible. Yet you don't want to be overly selective, which would make the filter brittle. There is a tool which can help you with this described later—the KMEANS helper.

Similarly the "B HSV" window shows the output from applying the six b-prefixed HSV ranges to the original, downsampled color image.

The "MERGED_HSV" window shows the resulting "OR" of the "A HSV" and "B HSV" windows.

Finally, the "SMEARED" output shows the results of applying the erode/dilate filters, the polygon formation, the canny filter to gather pixels into lines, turning the polygons into convex hulls over candidate cone areas and using the gausian blur filter. The only interesting detail in all of that is it shows the additional effet of the **poly_epsilon_** and **erode_kernel_size_** sliders. The other computations are (presently) hard coded. This is the important window. What you want to see here is an outline of a cone which is smaller at the top than the bottom and of the appropriate size. It doesn't hurt to have other artifacts in the window unless those artifacts end up looking like cones. The cone detector will reject polygons which are not of the right size and aspect ratio. Having lots of artifacts might slow down the detector, but having a brittle filter that doesn't work when the shading changes or the color temperature of the sun changes, a brittle filter that works on the first cone but not the last won't be valuable.

When you uncheck the **debug\_** option, the debug windows remain, showing the last state. You can close the windows as you like and they will be re-opened if you enable **debug\_** again.

----
# **The KMEANS helper**
In **rqt** if you use the **Services / Service Caller** plugin, you can invoke several useful services specific to Victoria. One of those is the service name **/cone_detector/calibrate_cone_detector**. You should see something like:

![Calibrate Cone Detection](/doc/calibrate_cone_detection.png)

To use this service, place the cone so that a non-specular part of the cone is in the exact center of the image and then click on the "Call" button. In the **rosout** window, you'll see something like:
~~~
There were 15617 pixels in the cluster and the new ranges are min_hue: 8, max_hue: 14, min_saturation: 195, max_saturation: 240, min_value: 226, max_value: 255
~~~
These show you HSV range values that you can plug into the cone detector dynamic reconfigure window (above) as starting points for one of the filters, typically the a-filter. Also as an aid, a window will pop up entitled "KMEANS_annotated". You should see the cone in the center and all of the pixels that were passed by the HSV filter will be colored pure black. Again, this gives you a starting point for setting your real HSV values. You could call this helper several times, each time with the robot showing the cone from a different angle or in different lighting to get an idea of the needed HSV filter values.

----
# **Invoking a strategy to achieve a goal**
Using **rqt** and the **Services / Service Caller** plugin, you can bring up a panel that allows you to inject one of the goals to be solved by the strategy module. It's the service **/robo_magellan_node/push_goal** and the panel looks like:

![Push Goal service](/doc/robomagellan_push_goal.png)

Each time you click on "Call", the goals which are set to **True** will be pushed onto the goal stack. If you also set the **execute_now** to **True**, after the goals are pushed the strategy module will atttempt to achieve them. You can push the same goal several times without execution if you leave **execute_now** set to **False**.  For instance, you can cause the robot to rotate a complete circle in place up to four times in a row if you push the **push_discover_cone** goal three times with **execute_now** set to **False** and then push it once more with **execute_now** set to **True**.

If more than one goal is pushed at a time, they are pushed in the order show, top to bottom. For example, if you set to **True** both the "push_move_to_cone" and "push discover_cone" and also the "execute_now", then the MoveToCone goal will be pushed onto the goal stack first, then the "DiscoverCone" goal is pushed, then the strategy module is told to achieve those goals. The result is that the robot will spin around until it detects the cone in the camera then attempt to move and touch the cone.

There is no way to clear out goals other than achieving all the goals or restarting the victoria_navigaton stack.

If you select the **push_seek_to_gps** then you must have also set up the required YAML file with a list of GPS wayspoints. You should also select the zero-based index into that list of the point you want to seek to via the **point_number_to_seek_to** text field. If the number you select is out of range, or if some other erroneous condition is detected, you'll see an error message in the response panel.

You can use this to repeatedly test a problem solver without invoking the whole RoboMagellan strategy.

Importantly, **to actually run the complete contest code, set push_solve_robomagellan and execute_now to True and click the Call button**.
