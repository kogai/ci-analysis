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

type diagnostic = (int, string);

let diagnostic_of_build = x => {
  let w = x.workflows;
  let sec =
    (x.start_time, x.stop_time)
    |> (
      ((a, b)) =>
        (MomentRe.moment(a), MomentRe.moment(b))
        |> (
          ((a, b)) =>
            MomentRe.diff(b, a, `seconds) |> Int32.of_float |> Int32.to_int
        )
    );
  let duration = Printf.sprintf("%dmin%dsec", sec / 60, sec mod 60);
  Printf.sprintf(
    "[%d:%s] %s/%s:%s(%s)",
    x.build_num,
    duration,
    x.username,
    x.reponame,
    x.branch,
    w.job_name,
  );
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
    xs => {
      Js.log(
        xs
        |> parseExn
        |> Array.to_list
        |> List.map(buildFromJs)
        |> List.map(x => {...x, workflows: workflowsFromJs(x.workflows)})
        |> List.fold_left(
             (acc, x) => {
               let ws =
                 switch (Belt.Map.String.get(acc, x.workflows.workflow_id)) {
                 | Some(ys) => [x, ...ys]
                 | None => [x]
                 };
               Belt.Map.String.set(acc, x.workflows.workflow_id, ws);
             },
             Belt.Map.String.empty,
           )
        |> Belt.Map.String.(
             forEach(
               _,
               (_, xs) => {
                 Js.log("");
                 List.iter(x => Js.log(diagnostic_of_build(x)), xs);
                 Js.log("");
               },
             )
           ),
      );
      return();
    }
  )
);
