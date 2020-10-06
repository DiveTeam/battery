import { SOC_CHANGE } from '../constants';

const initialState = {
  soc: 0,
};

const socReducer = (state = initialState, action) => {
  switch(action.type) {
  case SOC_CHANGE:
    return {
      ...state,
      soc: action.soc
    };
  default:
    return state;
  }
}
export default socReducer;