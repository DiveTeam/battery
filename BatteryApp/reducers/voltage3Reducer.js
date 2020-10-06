import { VOLTAGE3_CHANGE } from '../constants';

const initialState = {
  voltage3: 0,
};

const voltage3Reducer = (state = initialState, action) => {
  switch(action.type) {
  case VOLTAGE3_CHANGE:
    return {
      ...state,
      voltage3: action.voltage3
    };
  default:
    return state;
  }
}
export default voltage3Reducer;