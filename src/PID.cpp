#include "PID.h"

#define MAX_TUNE_STEPS 1000

using namespace std;

#define DEFAULT_AMOUNT_P 0.1
#define DEFAULT_AMOUNT_I 0.1
#define DEFAULT_AMOUNT_D 0.1
#define MAX_CTE 2.0
/*
 * TODO: Complete the PID class.
 */

PID::PID() {
//    is_initialized = false;
}

PID::~PID() {}

void PID::Init(double p, double i, double d, bool twiddle_mode_=false) {

    twiddle_mode = twiddle_mode_;

    Kpid[0] = p;
    Kpid[1] = i;
    Kpid[2] = d;

    TuneAmount[0] = DEFAULT_AMOUNT_P;
    TuneAmount[1] = DEFAULT_AMOUNT_I;
    TuneAmount[2] = DEFAULT_AMOUNT_D;

    ErrorPID[0] = 0.0; // error p
    ErrorPID[1] = 0.0; // error i
    ErrorPID[2] = 0.0; // error d
    total_square_error = 0;

    steps=0;
    tor = 0.02;
    best_avg_error = 99999;
    current_param_index = 0;

    if(twiddle_mode){
        twiddle_state = START;
    }
    tune_steps_num = 500;
}

void PID::UpdateError(double cte) {
    ErrorPID[2] = cte - ErrorPID[0]; // d
    ErrorPID[0] = cte; // p
    ErrorPID[1] += cte; // i
    steps++;

    //cout << "CTE: " << cte << std::endl;

//    if(steps < 3){
//        return;
//    }

//    if(!is_initialized){
//        is_initialized = true;
//    }
    if(twiddle_mode){
//        if(fabs(cte) > MAX_CTE){
//            cout << "Start tuning for " << tune_steps_num << " steps" << endl;
//            if(tune_steps_num == MAX_TUNE_STEPS+1){
//                tune_steps_num = steps;
//            }
//            if(steps > tune_steps_num){
//                tune_steps_num = steps;
//            }
//            twiddle_state = START;
//            Reset();
//            return;
//        }
//        if(steps == MAX_TUNE_STEPS){
//            cout << "sucessfully run a whole lap" << endl;
//            twiddle_mode = false;
//            return;
//        }
        Twiddle();
    }
}

double PID::TotalError() {
    return (-Kpid[0]*ErrorPID[0] - Kpid[1]*ErrorPID[1] - Kpid[2]*ErrorPID[2]);
}

void PID::Twiddle() {
    if(twiddle_state == STOP){
        return;
    }
    if(steps < tune_steps_num){
        total_square_error += (ErrorPID[0]*ErrorPID[0]);
    }else if(steps == tune_steps_num){
        double avg_err = total_square_error / steps;
        if((TuneAmount[0]+TuneAmount[1]+TuneAmount[2]) < tor){
            // stop
            cout << "Stop tuning for " << tune_steps_num << " steps" << endl;
            cout << "best avg error : " << avg_err << endl;
            cout << Kpid[0] << " " << Kpid[1] << " " << Kpid[2] << endl;
            twiddle_state = STOP;
        }else{
            if(twiddle_state == START){
                TryIncreaseCurrentParam();
            }else if(twiddle_state == TRY_INCREASE){
                double avg_err = total_square_error / steps;
                if(avg_err < best_avg_error){
                    best_avg_error = avg_err;
                    cout << "best avg error : " << best_avg_error << endl;
                    ScaleTunningAmount(1.1);
                    TryIncreaseNextParam();
                }else{
                    TryDecreaseCurrentParam();
                }
            }else if(twiddle_state == TRY_DECREASE){
                double avg_err = total_square_error / steps;
                if(avg_err < best_avg_error){
                    best_avg_error = avg_err;
                    cout << "best avg error : " << best_avg_error << endl;
                    ScaleTunningAmount(1.1);
                    TryIncreaseNextParam();
                }else{
                    ResetCurrentParam();
                    ScaleTunningAmount(0.9);
                    TryIncreaseNextParam();
                }
            }
        }
    }
}

void PID::SetWS(uWS::WebSocket<uWS::SERVER>* ws_)
{
    ws = ws_;
}

void PID::Reset()
{
    std::string reset_msg = "42[\"reset\",{}]";
    ws->send(reset_msg.data(), reset_msg.length(), uWS::OpCode::TEXT);
    ErrorPID[0] = 0.0;
    ErrorPID[1] = 0.0;
    ErrorPID[2] = 0.0;
    steps = 0;
    total_square_error = 0;
}

void PID::ScaleTunningAmount(double factor)
{
    TuneAmount[current_param_index] *= factor;
}

void PID::TryIncreaseCurrentParam()
{
    Kpid[current_param_index] += TuneAmount[current_param_index];
    twiddle_state = TRY_INCREASE;
    Reset();
    cout << "try params : " << Kpid[0] << ", " << Kpid[1] << ", " << Kpid[2] << endl;
    cout << "tune params : " << TuneAmount[0] << ", " << TuneAmount[1] << ", " << TuneAmount[2] << endl;
}

void PID::TryDecreaseCurrentParam()
{
    Kpid[current_param_index] -= 2*TuneAmount[current_param_index];
    twiddle_state = TRY_DECREASE;
    Reset();
    cout << "try params : " << Kpid[0] << ", " << Kpid[1] << ", " << Kpid[2] << endl;
    cout << "tune params : " << TuneAmount[0] << ", " << TuneAmount[1] << ", " << TuneAmount[2] << endl;
}

void PID::TryIncreaseNextParam()
{
    current_param_index = 0;
//    current_param_index++;
//    if(current_param_index == 3){
//        current_param_index = 0;
//    }
    Kpid[current_param_index] += TuneAmount[current_param_index];
    twiddle_state = TRY_INCREASE;
    Reset();
    cout << "try params : " << Kpid[0] << ", " << Kpid[1] << ", " << Kpid[2] << endl;
    cout << "tune params : " << TuneAmount[0] << ", " << TuneAmount[1] << ", " << TuneAmount[2] << endl;
}


void PID::ResetCurrentParam()
{
    Kpid[current_param_index] += TuneAmount[current_param_index];
}