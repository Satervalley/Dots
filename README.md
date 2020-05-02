# Dots
 Dots is a simple N-Body simulation system with visualization. 

* OS: Windows  
* Dev platform: Visual Studio comm 2019  
* Framework: MFC  
* Dependence: Boost
## Body types:
  There a two kinds of bodies:normal body(red, blue, green and purple) and Anti-G body(negative mass body, white and yellow), each kind has three types:
  - Common
  - Giant star
  - Blackhole(annular shape accretion disk)
  
## Performance
  Dots uses CPU for calculations. A simple test on a normal cpu (Intel 6200u) shows that:
  - Single thread with SIMD(AVX2)：about 3000 bodies at FPS 60
  - Multi threads with SIMD(AVX2)：about 4000 bodies at FPS 60
  
## Screen shots
![Common](../master/Screenshots/common.gif)
  
![Giant](../master/Screenshots/giant.gif)  
  
## License  

Copyright © 2020 Wang <wtcyh0707(at)gmail.com>
This work is free. You can redistribute it and/or modify it under the
terms of the Do What The Fuck You Want To Public License, Version 2,
as published by Sam Hocevar. See http://www.wtfpl.net/ for more details.  

<a href="http://www.wtfpl.net/"><img
       src="http://www.wtfpl.net/wp-content/uploads/2012/12/logo-220x1601.png"
       width="110" height="80" alt="WTFPL" /></a>
