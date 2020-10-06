import { applyMiddleware, createStore, combineReducers } from 'redux';
import voltage1Reducer from '../reducers/voltage1Reducer';
import voltage2Reducer from '../reducers/voltage2Reducer';
import voltage3Reducer from '../reducers/voltage3Reducer';
import currentReducer from '../reducers/currentReducer';
import temperatureReducer from '../reducers/temperatureReducer';
import socReducer from '../reducers/socReducer';
import { createLogger } from 'redux-logger'


const logger = createLogger({
  level: 'log'
});

const rootReducer = combineReducers(
  { voltage1: voltage1Reducer,
    voltage2: voltage2Reducer,
    voltage3: voltage3Reducer,
    current: currentReducer,
    temperature: temperatureReducer,
    soc: socReducer,
   }
);

const configureStore = () => {
  return createStore(rootReducer, applyMiddleware(logger));
}

export default configureStore;