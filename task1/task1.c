#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (){
    FILE * fptr;
    double x, y;//  store x and y values from the dataset
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0; //  store sums used in linear regression
    int n = 0;
    fptr = fopen("datasetLR1.txt", "r");// atidaro faila
	
    if (fptr == NULL) {// chake or file is not emmpty
        printf("Could not open file.\n");
        return 1;
        }
// Read the data points x, y from the file till the end
    while (fscanf(fptr, "%lf,%lf", &x, &y) == 2) {
        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        n++;
    }
    fclose(fptr);// Close the file po perskaitimo viso data
    
	// Calculate the denominator for the linear regression formula
    double denominator = n * sumX2 - sumX * sumX;
    
    // apskaicuoti (A) and slope (B)  linear regression line
    double A = (sumY * sumX2 - sumX * sumXY) / denominator;
    double B = (n * sumXY - sumX * sumY) / denominator;	
    printf("The linear equation is: y = %.2fx + %.2f\n", B, A);// form y = Bx + A
  
    double inputX, predictedY;
    printf("Enter a value for x to predict y: ");
    scanf("%lf", &inputX);
	

    predictedY = B * inputX + A;//predicted y value using the regression line equation
    printf("For x = %.2f, the predicted y is: %.2f\n", inputX, predictedY);  


    
	return 0;
}

//cd 

//gcc -o task1 task1.c -lm
//./task1


