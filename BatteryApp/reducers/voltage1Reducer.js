import { VOLTAGE1_CHANGE } from '../constants';

const initialState = {
  voltage1: 0,
};

const voltage1Reducer = (state = initialState, action) => {
  switch(action.type) {
  case VOLTAGE1_CHANGE:
    return {
      ...state,
      voltage1: action.voltage1
    };
  default:
    return state;
  }
}
export default voltage1Reducer;