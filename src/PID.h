#ifndef PID_H
#define PID_H

#include <uWS/uWS.h>

class PID {
public:

//    bool is_initialized;
    bool twiddle_mode;
    unsigned int tune_steps_num;
    uWS::WebSocket<uWS::SERVER>* ws;

    unsigned int current_param_index;
    double tor = 0.2; // tolerance for twiddle
    /*
     * Errors
     */
    double ErrorPID[3];
    double TuneAmount[3];
//    double p_error;
//    double i_error;
//    double d_error;

    /*
     * Coefficients
     */ 
    double Kpid[3];
//    double Kp;
//    double Ki;
//    double Kd;

    /*
     * Constructor
     */
    PID();

    /*
     * Destructor.
     */
    virtual ~PID();

    /*
     * Initialize PID.
     */
    void Init(double p, double i, double d, bool twiddle_mode_);

    /*
     * Update the PID error variables given cross track error.
     */
    void UpdateError(double cte);

    /*
     * Calculate the total PID error.
     */
    double TotalError();

    void SetWS(uWS::WebSocket<uWS::SERVER>* ws_);

private:
    enum TwiddleState {START,STOP,TRY_INCREASE,TRY_DECREASE};
    TwiddleState twiddle_state;
    double total_error;
    double total_square_error;
    double best_avg_error;
    unsigned int steps;
    void Twiddle();
    void Reset();
    void ScaleTunningAmount(double factor);
    void TryIncreaseCurrentParam();
    void TryDecreaseCurrentParam();
    void TryIncreaseNextParam();
    void ResetCurrentParam();
};


#endif /* PID_H */
