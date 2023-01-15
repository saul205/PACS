__kernel void flip(
	__global unsigned char* img,
	int height, int width, int length){

	int i = get_global_id(0);
	
	if(i >= length)
		return;

	int w2 = width / 2;

	unsigned char aux = img[i % w2 + i / w2 * width];
	img[i % w2 + i / w2 * width] = img[(width - 1 - i % w2) + i / w2 * width];
	img[(width - 1 - i % w2) + i / w2 * width] = aux;
}