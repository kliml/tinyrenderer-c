float d_sqrt(float number)
{
    int i;
    float x, y;
    x = number * 0.5;
    y  = number;
    i  = * (int * ) &y;
    i  = 0x5f3759df - (i >> 1);
    y  = * ( float * ) &i;
    y  = y * (1.5 - (x * y * y));
    y  = y * (1.5 - (x * y * y));
    return number * y;
}

void vec_sub3f(float *output, float *a, float *b) {
    output[0] = a[0] - b[0];
    output[1] = a[1] - b[1];
    output[2] = a[2] - b[2];
}

void vec_xor3f(float *output, float *a, float *b) {
    output[0] = a[1] * b[2] - a[2] * b[1];
    output[1] = a[2] * b[0] - a[0] * b[2];
    output[2] = a[0] * b[1] - a[1] * b[0];
}

void vec_cross3f(float *output, float *a, float *b)
{
	output[0] = a[1] * b[2] - a[2] * b[1];
	output[1] = a[2] * b[0] - a[0] * b[2];
	output[2] = a[0] * b[1] - a[1] * b[0];
}

void vec_normalize3f(float *vec)
{
	float f;
	f = d_sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= f;
	vec[1] /= f;
	vec[2] /= f;
}

float vec_dot3f(float *a, float *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vec_cross3i(int *output, int *a, int *b)
{
    output[0] = a[1] * b[2] - a[2] * b[1];
    output[1] = a[2] * b[0] - a[0] * b[2];
    output[2] = a[0] * b[1] - a[1] * b[0];
}