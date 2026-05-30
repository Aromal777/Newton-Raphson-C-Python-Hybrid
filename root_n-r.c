#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <Python.h>
#include "tinyexpr.h"
int main(){ 
    char func[256];
    char fprime_str[512];
    double x0,x1;
    double tolerance;
    int iteration =0;
    printf("Enter the function (in Python syntax, e.g., x**2 - 3*x + 2): ");
    fgets(func, sizeof(func), stdin);
    func[strcspn(func, "\n")] = 0;  // remove newline
    printf("Enter the Tolerance: ");
    scanf("%lf",&tolerance);
    printf("Enter initial guess x0: ");
    scanf("%lf", &x0);
    Py_Initialize();
    PyRun_SimpleString(
    "import sys\n"
    "import sympy\n"
    );
    PyRun_SimpleString("print('Sympy imported successfully!')");
    #define Pystring PyRun_SimpleString      //renaming pyrun_simplestring function to pystring
    #define PyImport PyImport_AddModule     //renaming 
    Pystring("from sympy import symbols,diff,lambdify, sympify,sin,cos,tan,log,exp");
    Pystring("x = symbols('x')");
    char command[10000];
    sprintf(command, "f = sympify('%s')\n",func);   
    Pystring(command);
    Pystring("fprime = diff(f,x)\n");
    Pystring("fprime_str = str(fprime)\n");
    Pystring("print(\"f'(x)= \", fprime)");
    Pystring("import math \n"
    "f_func = lambdify(x,f,['math'])\n" //f_func defined in __main__ module
    "fprime_func = lambdify(x,fprime,['math'])");
    PyObject *main_module = PyImport_AddModule("__main__");
    PyObject *globals = PyModule_GetDict(main_module);
    PyObject *fprime_py = PyDict_GetItemString(globals, "fprime_str");
    const char *fprime_tmp = PyUnicode_AsUTF8(fprime_py);
    strcpy(fprime_str, fprime_tmp);
    printf("\nf(x) = %s", func);
    printf("\nf'(x) = %s\n", fprime_str);
    printf("\nPython derivative done. Starting C iteration\n");
    Py_Finalize();
    for (int i = 0; func[i]; i++) {   //loop because tinyexpr takes ** as ^
        if (func[i] == '*' && func[i+1] == '*') {
            func[i] = '^';
            memmove(&func[i+1], &func[i+2], strlen(&func[i+2]) + 1);
        }
    }
    for (int i = 0; fprime_str[i]; i++) {
        if (fprime_str[i] == '*' && fprime_str[i+1] == '*') {
            fprime_str[i] = '^';
            memmove(&fprime_str[i+1], &fprime_str[i+2], strlen(&fprime_str[i+2]) + 1);
        }
    }
    printf("\nNewton-Raphson Iterations:\n");
    double fx, fpx;
    te_expr *expr_f, *expr_fp; 
    //compiled expressional object representing f(x)
    te_variable vars[] = { {"x", &x0} }; 
    // defines the variable x in the expression and points the value of x0
     do {
        expr_f = te_compile(func, vars, 1, 0); 
        //{te_compile(string func , variable arr , no. of variables, pointer to an error position)}
        fx = te_eval(expr_f); 
        //evaluates the function with the value of x0
        te_free(expr_f); 
        //frees the memory used by expr_f
        expr_fp = te_compile(fprime_str, vars, 1, 0);
        fpx = te_eval(expr_fp);
        te_free(expr_fp);
        if (fpx == 0) {
            printf("Derivative zero at x = %lf\n", x0);
            break;
        }
        x1 = x0 - fx / fpx;
        printf("Iter %2d: x = %.10lf\n", iteration + 1, x1);
        if (fabs(x1 - x0) < tolerance) {
            printf("Converged within tolerance after %d iterations.\n", iteration + 1);
            break;
        }
        x0 = x1;
        iteration++;
    } while (iteration < 100);
    printf("\nApproximate Root = %lf\n", x1);
    return 0;
}