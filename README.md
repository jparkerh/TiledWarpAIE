# TiledWarpAIE

The tiling warp is currently built out with a 4-tile configuration. Operates on 24-bit monochrome pixels, each pixel is aligned to 32-bits in the input stream (8-bit padding). Each tile has 32KB of memory, so each instantiation of the kernel has a stack with the data memory for that tile allocated statically:

```
uint32_t data[7068];
```

## Theory of operation

<img width="989" alt="Screen Shot 2021-06-13 at 11 10 44 PM" src="https://user-images.githubusercontent.com/42722137/121838078-a0d1ca80-cc9c-11eb-9e63-bb54496e7d62.png">

 1. Using 4 tiles, sends pixels along the chain
 2. Each tile stores it's pixels to memory
 3. Each tile performs the compute, outputting
 	the values that it can via its stored data
 4. Streaming HLS function reorders output pixels
 	in the programmable logic
 
 ## Real Time Ports
 
 ### Range Descriptor
 <img width="487" alt="Screen Shot 2021-06-13 at 11 13 00 PM" src="https://user-images.githubusercontent.com/42722137/121838190-e7bfc000-cc9c-11eb-8a48-4985b33012ee.png">
 Each tile has a RangeDescriptor real time port, through which we control what pixels filter into that tile's local memory.
 
 ### Warp Coefficients
 <img width="378" alt="Screen Shot 2021-06-13 at 11 15 24 PM" src="https://user-images.githubusercontent.com/42722137/121838349-3e2cfe80-cc9d-11eb-8dd3-0b85f0f5fd87.png">
 Each tile has a Warp Coefficients real time port to control the operation of the warp function. Can be broadcast across all tiles.
