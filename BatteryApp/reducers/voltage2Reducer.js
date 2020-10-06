import { VOLTAGE2_CHANGE } from '../constants';

const initialState = {
  voltage2: 0,
};

const voltage2Reducer = (state = initialState, action) => {
  switch(action.type) {
  case VOLTAGE2_CHANGE:
    return {
      ...state,
      voltage2: action.voltage2
    };
  default:
    return state;
  }
}
export default voltage2Reducer;
