[%raw "require('isomorphic-fetch')"];

type iso8601 = string;

[@bs.deriving jsConverter]
type workflows = {
  job_name: string,
  workflow_id: string,
};

[@bs.deriving jsConverter]
type build('a) = {
  build_num: int,
  reponame: string,
  username: string,
  branch: string,
  subject: string,
  status: [ | `sucess],
  build_time_millis: int,
  committer_date: iso8601,
  usage_queued_at: iso8601,
  start_time: iso8601,
  stop_time: iso8601,
  workflows: 'a,
};

/* let endpoint = "https://circleci.com/api/v1.1/me"; */
let endpoint = "https://circleci.com/api/v1.1/recent-builds";

/* GET: /project/:vcs-type/:username/:project/:build_num  */
/* let endpoint = "https://circleci.com/api/v1.1/project/github/feedforce/ecbooster/17931"; */
[@bs.val] external token : string = "process.env.CI_TOKEN";

exception Error;

[@bs.val] [@bs.scope "JSON"] external parseExn : string => 'a = "parse";

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
      Js.log(x |> parseExn |> Array.map(buildFromJs));
      return();
    }
  )
);
