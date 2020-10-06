import { TEMPERATURE_CHANGE } from '../constants';

const initialState = {
  temperature: 0,
};

const temperatureReducer = (state = initialState, action) => {
  switch(action.type) {
  case TEMPERATURE_CHANGE:
    return {
      ...state,
      temperature: action.temperature
    };
  default:
    return state;
  }
}
export default temperatureReducer;