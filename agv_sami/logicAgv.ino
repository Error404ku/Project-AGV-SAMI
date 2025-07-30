void agvMode(AgvState state){
    if(state == AGV_STATE_MOVE_FORWARD){
        agvMoveForward();
    }
    else if(state == AGV_STATE_MOVE_BACKWARD){
        agvMoveBackward();
    }
    else if(state == AGV_STATE_STOP){
        agvStop();
    }
    else if(state == AGV_STATE_TERMINAL){
        agvTerminal();
    }
    else if(state == AGV_STATE_WAREHOUSE){
        agvWarehouse();
    }
    else if(state == AGV_STATE_STATION){
        agvStation();
    } else{
        agvStop();
    }
}

void agvWarehouse(){
    agvStop();
    if (START()){
        agvMode(AGV_STATE_MOVE_FORWARD);
    }
}

void agvStation(){
    agvStop();
    if (START()){
        agvMode(AGV_STATE_MOVE_FORWARD);
    }
}

void agvTerminal(){
    agvStop();
    if (START()){
        agvMode(AGV_STATE_MOVE_FORWARD);
    }
}

void agvStop(){
    pidLinefollower(2,PID_MODE_STOPPELANPELAN);
}       

void agvMoveForward(){
    if (!obstacleDetected){
        pidLinefollower(2,PID_MODE_MAJU);
    }
    else{
        agvStop();
    }
}
