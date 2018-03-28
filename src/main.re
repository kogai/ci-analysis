[%raw "require('isomorphic-fetch')"];

type payload = {. "circle-token": string};

let endpoint = "https://circleci.com/api/v1.1/me";

[@bs.val] external token : string = "process.env.CI_TOKEN";

exception Error;

Async.(
  Js.Json.stringifyAny({"circle-token": token})
  |> Js.Option.getExn
  |> (
    x =>
      Fetch.RequestInit.make(
        ~method_=Fetch.Get,
        ~body=Fetch.BodyInit.make(x),
        (),
      )
  )
  |> Fetch.fetchWithInit(
       Printf.sprintf("%s?circle-token=%s", endpoint, token),
     )
  >>= Fetch.Response.text
  >>= (
    x => {
      Js.log(x);
      return();
    }
  )
);
